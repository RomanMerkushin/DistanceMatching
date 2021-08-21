// Copyright Roman Merkushin. All Rights Reserved.

#include "GameFramework/DistanceMatchingComponent.h"
#include "Log.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#if ENABLE_DRAW_DEBUG
TAutoConsoleVariable<int32> CVarDistanceMatchingComponentDebug(TEXT("c.DistanceMatching.Debug"), 0, TEXT("Turn on debug for DistanceMatching component"));
#endif

UDistanceMatchingComponent::UDistanceMatchingComponent()
	: bIsMoving(false)
	, bIsAccelerating(false)
	, DistanceToMarker(0.0f)
	, DistanceMatchingType(EDistanceMatchingType::None)
	, ActorLocation(FVector::ZeroVector)
	, PreviousActorLocation(FVector::ZeroVector)
	, Velocity(FVector::ZeroVector)
	, Acceleration(FVector::ZeroVector)
	, PreviousAccelerationSize(0.0f)
	, MarkerLocation(FVector::ZeroVector)
	, MaxSimulationTime(2.0f)
	, MaxDistanceToMarker(100.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

FVector UDistanceMatchingComponent::PredictStopLocation(const float DeltaTime) const
{
	const float MinTickTime = MovementComponent->MIN_TICK_TIME;
	const float SimulationTimeStep = FMath::Min(DeltaTime, MovementComponent->MaxSimulationTimeStep * Character->GetActorTimeDilation());
	// Subdivide braking to get reasonably consistent results at lower frame rates
	// (important for packet loss situations w/networking)
	const float MaxTimeStep = FMath::Clamp(MovementComponent->BrakingSubStepTime, 1.0f / 75.0f, 1.0f / 20.0f);
	const float ActualBrakingFriction = MovementComponent->bUseSeparateBrakingFriction ? MovementComponent->BrakingFriction : MovementComponent->GroundFriction;
	const float FrictionFactor = FMath::Max(0.0f, MovementComponent->BrakingFrictionFactor);
	const float Friction = FMath::Max(0.0f, ActualBrakingFriction * FrictionFactor);
	const float BrakingDeceleration = FMath::Max(0.0f, MovementComponent->GetMaxBrakingDeceleration());
	const bool bZeroFriction = Friction == 0.0f;
	const bool bZeroBraking = BrakingDeceleration == 0.0f;
	const float BrakeToStopVelocity = MovementComponent->BRAKE_TO_STOP_VELOCITY;
	const bool bZeroAcceleration = Acceleration.IsZero();

	FVector PredictedVelocity = Velocity;
	FVector PredictedLocation = ActorLocation;
	float PredictionTime = 0.0f;

	while (MaxSimulationTime > PredictionTime)
	{
		const FVector PreviousVelocity = PredictedVelocity;

		// Apply velocity braking
		if (bZeroAcceleration)
		{
			if (PredictedVelocity.IsZero() || bZeroFriction && bZeroBraking || SimulationTimeStep < MinTickTime)
			{
				return PredictedLocation;
			}

			float RemainingTime = SimulationTimeStep;
			// Decelerate to brake to a stop
			const FVector ReverseAcceleration = bZeroBraking ? FVector::ZeroVector : -BrakingDeceleration * PredictedVelocity.GetSafeNormal();

			while (RemainingTime >= MinTickTime)
			{
				// Zero friction uses constant deceleration, so no need for iteration
				const float DT = RemainingTime > MaxTimeStep && !bZeroFriction ? FMath::Min(MaxTimeStep, RemainingTime * 0.5f) : RemainingTime;
				RemainingTime -= DT;

				// Apply friction and braking
				PredictedVelocity = PredictedVelocity + (-Friction * PredictedVelocity + ReverseAcceleration) * DT;

				// Don't reverse direction
				if ((PredictedVelocity | PreviousVelocity) <= 0.0f)
				{
					break;
				}
			}

			// Clamp to zero if nearly zero, or if below min threshold and braking
			const float VelocitySizeSquared = PredictedVelocity.SizeSquared();
			if (VelocitySizeSquared <= KINDA_SMALL_NUMBER || !bZeroBraking && VelocitySizeSquared <= FMath::Square(BrakeToStopVelocity))
			{
				return PredictedLocation;
			}
		}
		else
		{
			// Friction affects our ability to change direction. This is only done for input acceleration, not path following.
			const FVector AccelerationDirection = Acceleration.GetSafeNormal();
			const float Speed = PredictedVelocity.Size();
			PredictedVelocity = PredictedVelocity - (PredictedVelocity - AccelerationDirection * Speed) * FMath::Min(DeltaTime * Friction, 1.0f);

			// Apply additional requested acceleration
			PredictedVelocity += Acceleration * DeltaTime;
		}

		PredictedLocation += PredictedVelocity * SimulationTimeStep;
		PredictionTime += SimulationTimeStep;

		if ((PredictedVelocity | PreviousVelocity) <= 0.0f)
		{
			return PredictedLocation;
		}
	}

	return PredictedLocation;
}

void UDistanceMatchingComponent::InitializeComponent()
{
	Super::InitializeComponent();

	Character = Cast<ACharacter>(GetOwner());
	MovementComponent = IsValid(Character) ? Character->GetCharacterMovement() : nullptr;

	if (!MovementComponent)
	{
		UE_LOG(LogDistanceMatching, Error, TEXT("Can't access CharacterMovement component. DistanceMatching plugin will be disabled!"));
		PrimaryComponentTick.bCanEverTick = false;
		return;
	}

	ActorLocation = Character->GetActorLocation();
	PreviousActorLocation = ActorLocation;
}

void UDistanceMatchingComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ActorLocation = Character->GetActorLocation();
	Velocity = MovementComponent->Velocity;
	Acceleration = MovementComponent->GetCurrentAcceleration();

	const float VelocitySize = Velocity.Size();
	const float AccelerationSize = Acceleration.Size();

	bIsMoving = VelocitySize > DELTA;
	bIsAccelerating = AccelerationSize > DELTA;

#if ENABLE_DRAW_DEBUG
	const bool bShowDebug = CVarDistanceMatchingComponentDebug.GetValueOnAnyThread() == 1;
	const TObjectPtr<UWorld> World = GetWorld();

	if (bShowDebug)
	{
		DrawDebugSphere(World, ActorLocation, 16.0f, 16.0f, FColor::Green, false, -1.0f, 0, 0.3f);
	}
#endif

	if (bIsAccelerating)
	{
		if (DistanceMatchingType != EDistanceMatchingType::Start && (Velocity | Acceleration) > 0.0f && bIsMoving)
		{
			DistanceMatchingType = EDistanceMatchingType::Start;
			if (PreviousAccelerationSize < DELTA)
			{
				MarkerLocation = PreviousActorLocation;
				OnMoving.Broadcast(this);

#if ENABLE_DRAW_DEBUG
				if (bShowDebug)
				{
					DrawDebugSphere(World, MarkerLocation, 16.0f, 16.0f, FColor::Orange, false, 1.5f, 0, 0.3f);
				}
#endif
			}
		}
		else if (DistanceMatchingType != EDistanceMatchingType::Pivot && (Velocity | Acceleration) <= 0.0f)
		{
			DistanceMatchingType = EDistanceMatchingType::Pivot;
			MarkerLocation = PredictStopLocation(DeltaTime);
			OnPivoting.Broadcast(this);

#if ENABLE_DRAW_DEBUG
			if (bShowDebug)
			{
				DrawDebugSphere(World, MarkerLocation, 16.0f, 16.0f, FColor::Purple, false, 1.5f, 0, 0.3f);
			}
#endif
		}
	}
	else if (DistanceMatchingType != EDistanceMatchingType::Stop && bIsMoving && !bIsAccelerating)
	{
		DistanceMatchingType = EDistanceMatchingType::Stop;
		MarkerLocation = PredictStopLocation(DeltaTime);
		OnStopping.Broadcast(this);
	}
	else if (!bIsMoving && !bIsAccelerating)
	{
		DistanceMatchingType = EDistanceMatchingType::None;
	}

#if ENABLE_DRAW_DEBUG
	if (bShowDebug)
	{
		if (DistanceMatchingType == EDistanceMatchingType::Stop)
		{
			DrawDebugSphere(World, MarkerLocation, 16.0f, 16.0f, FColor::Red, false, -1.0f, 0, 0.3f);
		}
		if (bIsMoving)
		{
			DrawDebugLine(World, PreviousActorLocation, ActorLocation, FColor::Cyan, false, 2.0f, 0, 0.75f);
		}
	}
#endif

	DistanceToMarker = DistanceMatchingType == EDistanceMatchingType::None
						? 0.0f
						: FMath::Clamp(FVector::Distance(ActorLocation, MarkerLocation), -MaxDistanceToMarker, MaxDistanceToMarker);

	if (DistanceMatchingType == EDistanceMatchingType::Stop || DistanceMatchingType == EDistanceMatchingType::Pivot)
	{
		DistanceToMarker *= -1.0f;
	}

	PreviousActorLocation = ActorLocation;
	PreviousAccelerationSize = AccelerationSize;
}
