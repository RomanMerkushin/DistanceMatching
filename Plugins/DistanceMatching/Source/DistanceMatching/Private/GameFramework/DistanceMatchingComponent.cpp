// Copyright Roman Merkushin. All Rights Reserved.

#include "GameFramework/DistanceMatchingComponent.h"
#include "Log.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#if ENABLE_DRAW_DEBUG
namespace DistanceMatchingCVars
{
	static int32 Debug = 0;
	FAutoConsoleVariableRef CVarDebug(
		TEXT("c.DistanceMatching.Debug"),
		Debug,
		TEXT("Turn on debug for DistanceMatching component."),
		ECVF_Default);

	static int32 DrawDebugTrace = 0;
	FAutoConsoleVariableRef CVarDrawDebugTrace(
		TEXT("c.DistanceMatching.DrawDebugTrace"),
		DrawDebugTrace,
		TEXT("Turn on draw debug trace for DistanceMatching component."),
		ECVF_Default);
}  // namespace DistanceMatchingCVars
#endif

UDistanceMatchingComponent::UDistanceMatchingComponent()
	: ActorLocation(ForceInitToZero)
	, PreviousActorLocation(ForceInitToZero)
	, Velocity(ForceInitToZero)
	, VelocitySize(0.0f)
	, Acceleration(ForceInitToZero)
	, AccelerationSize(0.0f)
	, PreviousAccelerationSize(0.0f)
	, CapsuleRadius(0.0f)
	, CapsuleHalfHeight(0.0f)
	, DistanceToFloor(0.0f)
	, GravityZ(0.0f)
	, bShowDebug(false)
	, bDrawDebugTrace(false)
	, bIsMoving(false)
	, bIsAccelerating(false)
	, bIsFalling(false)
	, DistanceMatchingType(EDistanceMatchingType::None)
	, MarkerLocation(ForceInitToZero)
	, DistanceToMarker(0.0f)
	, TimeToMarker(0.0f)
	, MaxSimulationTime(2.0f)
	, ApexSimulationFrequency(5.0f)
	, LandingSimulationFrequency(5.0f)
	, MinPivotAngle(150.0f)
	, TraceChannel(TraceTypeQuery1)
	, StopLocationTraceHalfHeight(150.0f)
	, DebugSphereRadius(16.0f)
	, DebugDrawTime(1.5f)
	, TraceDrawTime(2.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void UDistanceMatchingComponent::InitializeComponent()
{
	Super::InitializeComponent();

	World = GetWorld();

	if (!World)
	{
		UE_LOG(LogDistanceMatching, Error, TEXT("Can't access World. DistanceMatching plugin will be disabled!"));
		PrimaryComponentTick.bCanEverTick = false;
		return;
	}

	Character = Cast<ACharacter>(GetOwner());

	MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
	CapsuleComponent = Character ? Character->GetCapsuleComponent() : nullptr;

	if (!MovementComponent || !CapsuleComponent)
	{
		UE_LOG(LogDistanceMatching, Error, TEXT("Can't access character components. DistanceMatching plugin will be disabled!"));
		PrimaryComponentTick.bCanEverTick = false;
		return;
	}

	ActorLocation = Character->GetActorLocation();
	PreviousActorLocation = ActorLocation;
}

void UDistanceMatchingComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update character essential values
	PreviousActorLocation = ActorLocation;
	ActorLocation = Character->GetActorLocation();
	Velocity = Character->GetVelocity();
	VelocitySize = Velocity.Size();
	PreviousAccelerationSize = AccelerationSize;
	Acceleration = MovementComponent->GetCurrentAcceleration();
	AccelerationSize = Acceleration.Size();
	CapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	CapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	DistanceToFloor = MovementComponent->CurrentFloor.FloorDist;
	GravityZ = MovementComponent->GetGravityZ();

	// Update character movement states
	bIsMoving = VelocitySize > MOVEMENT_THRESHOLD;
	bIsAccelerating = AccelerationSize > MOVEMENT_THRESHOLD;
	bIsFalling = MovementComponent->IsFalling();

#if ENABLE_DRAW_DEBUG
	bShowDebug = DistanceMatchingCVars::Debug == 1;
	bDrawDebugTrace = DistanceMatchingCVars::DrawDebugTrace == 1;

	if (bShowDebug)
	{
		DrawDebugSphere(World, ActorLocation, DebugSphereRadius, 16.0f, FColor::Green, false, -1.0f, 0, 0.3f);
	}
#endif

	if (DistanceMatchingType != EDistanceMatchingType::Jump && bIsFalling && Velocity.Z > 0.0f)
	{
		DistanceMatchingType = EDistanceMatchingType::Jump;

		FPredictResult PredictApexResult;
		PredictJumpApex(PredictApexResult);

		MarkerLocation = PredictApexResult.Location;
		TimeToMarker = PredictApexResult.Time;

#if ENABLE_DRAW_DEBUG
		if (bShowDebug)
		{
			DrawDebugSphere(World, PreviousActorLocation, DebugSphereRadius, 16.0f, FColor::Green, false, DebugDrawTime, 0, 0.3f);
			DrawDebugSphere(World, MarkerLocation, DebugSphereRadius, 16.0f, FColor::Purple, false, DebugDrawTime, 0, 0.3f);
		}
#endif
	}
	else if (DistanceMatchingType != EDistanceMatchingType::Fall && bIsFalling && Velocity.Z < 0.0f)
	{
		DistanceMatchingType = EDistanceMatchingType::Fall;

		FPredictResult PredictLandingResult;
		PredictLandingLocation(PredictLandingResult);

		MarkerLocation = PredictLandingResult.Location;
		TimeToMarker = PredictLandingResult.Time;
	}
	else if (!bIsFalling)
	{
		if (bIsAccelerating)
		{
			if (DistanceMatchingType != EDistanceMatchingType::Start && (Velocity | Acceleration) > 0.0f && bIsMoving)
			{
				DistanceMatchingType = EDistanceMatchingType::Start;
				if (PreviousAccelerationSize < MOVEMENT_THRESHOLD)
				{
					MarkerLocation = PreviousActorLocation;
					TimeToMarker = 0.0f;

#if ENABLE_DRAW_DEBUG
					if (bShowDebug)
					{
						DrawDebugSphere(World, MarkerLocation, DebugSphereRadius, 16.0f, FColor::Orange, false, DebugDrawTime, 0, 0.3f);
					}
#endif
				}
			}
			else if (DistanceMatchingType != EDistanceMatchingType::Pivot && (Velocity.GetSafeNormal() | Acceleration.GetSafeNormal()) <= -(MinPivotAngle / 180.0f))
			{
				DistanceMatchingType = EDistanceMatchingType::Pivot;

				FPredictResult PredictPivotResult;
				PredictStopLocation(PredictPivotResult, DeltaTime);

				MarkerLocation = PredictPivotResult.Location;
				TimeToMarker = PredictPivotResult.Time;

#if ENABLE_DRAW_DEBUG
				if (bShowDebug)
				{
					DrawDebugSphere(World, MarkerLocation, DebugSphereRadius, 16.0f, FColor::Purple, false, DebugDrawTime, 0, 0.3f);
				}
#endif
			}
		}
		else if (DistanceMatchingType != EDistanceMatchingType::Stop && bIsMoving && !bIsAccelerating)
		{
			DistanceMatchingType = EDistanceMatchingType::Stop;

			FPredictResult PredictStopResult;
			PredictStopLocation(PredictStopResult, DeltaTime);

			MarkerLocation = PredictStopResult.Location;
			TimeToMarker = PredictStopResult.Time;
		}
		else if (!bIsMoving && !bIsAccelerating)
		{
			DistanceMatchingType = EDistanceMatchingType::None;
		}
	}

#if ENABLE_DRAW_DEBUG
	if (bShowDebug)
	{
		if (DistanceMatchingType == EDistanceMatchingType::Stop || DistanceMatchingType == EDistanceMatchingType::Fall)
		{
			DrawDebugSphere(World, MarkerLocation, DebugSphereRadius, 16.0f, FColor::Red, false, -1.0f, 0, 0.3f);
		}
		if (bIsMoving)
		{
			DrawDebugLine(World, PreviousActorLocation, ActorLocation, FColor::Cyan, false, DebugDrawTime, 0, 0.75f);
		}
	}
#endif

	// Update distance to marker
	DistanceToMarker = DistanceMatchingType == EDistanceMatchingType::None
						   ? 0.0f
						   : FMath::Clamp(FVector::Distance(ActorLocation, MarkerLocation), -MAX_MARKER_VALUE, MAX_MARKER_VALUE);

	if (DistanceMatchingType == EDistanceMatchingType::Stop || DistanceMatchingType == EDistanceMatchingType::Pivot)
	{
		DistanceToMarker *= -1.0f;
	}

	// Update time to marker
	switch (DistanceMatchingType)
	{
		case EDistanceMatchingType::None:
			TimeToMarker = 0.0f;
			break;
		case EDistanceMatchingType::Start:
			TimeToMarker += DeltaTime;
			break;
		default:
			TimeToMarker = FMath::Clamp(TimeToMarker - DeltaTime, 0.0f, MAX_MARKER_VALUE);
			break;
	}
}

void UDistanceMatchingComponent::PredictStopLocation(FPredictResult& PredictResult, const float DeltaTime) const
{
	const float FrictionFactor = FMath::Max(0.0f, MovementComponent->BrakingFrictionFactor);
	const float Friction = FMath::Max(0.0f, MovementComponent->GroundFriction * FrictionFactor);
	const float BrakingDeceleration = FMath::Max(0.0f, MovementComponent->GetMaxBrakingDeceleration());
	const bool bZeroFriction = Friction == 0.0f;
	const bool bZeroBraking = BrakingDeceleration == 0.0f;
	const float BrakeToStopVelocity = MovementComponent->BRAKE_TO_STOP_VELOCITY;
	const bool bZeroAcceleration = Acceleration.IsZero();

	FVector PredictedVelocity = bZeroAcceleration ? Velocity : Velocity.ProjectOnToNormal(Acceleration.GetSafeNormal());
	FVector PredictedLocation = ActorLocation;
	float PredictionTime = 0.0f;

	while (MaxSimulationTime > PredictionTime)
	{
		const FVector PreviousVelocity = PredictedVelocity;
		const float SimulationTimeStep = FMath::Min(MaxSimulationTime - DeltaTime, DeltaTime);

		// Apply velocity braking
		if (bZeroAcceleration)
		{
			if (PredictedVelocity.IsZero() || bZeroFriction && bZeroBraking)
			{
				break;
			}

			// Decelerate to brake to a stop
			const FVector ReverseAcceleration = bZeroBraking ? FVector::ZeroVector : -BrakingDeceleration * PredictedVelocity.GetSafeNormal();

			// Apply friction and braking
			PredictedVelocity = PredictedVelocity + (-Friction * PredictedVelocity + ReverseAcceleration) * SimulationTimeStep;

			// Clamp to zero if nearly zero, or if below min threshold and braking
			const float VelocitySizeSquared = PredictedVelocity.SizeSquared();
			if (VelocitySizeSquared <= KINDA_SMALL_NUMBER || !bZeroBraking && VelocitySizeSquared <= FMath::Square(BrakeToStopVelocity))
			{
				break;
			}
		}
		else
		{
			// Friction affects our ability to change direction. This is only done for input acceleration, not path following.
			const FVector AccelerationDirection = Acceleration.GetSafeNormal();
			const float Speed = PredictedVelocity.Size();
			PredictedVelocity = PredictedVelocity - (PredictedVelocity - AccelerationDirection * Speed) * FMath::Min(SimulationTimeStep * Friction, 1.0f);

			// Apply additional requested acceleration
			PredictedVelocity += Acceleration * SimulationTimeStep;
		}

		// Don't reverse direction
		if ((PredictedVelocity | PreviousVelocity) <= 0.0f)
		{
			break;
		}

		PredictedLocation += PredictedVelocity * SimulationTimeStep;
		PredictionTime += SimulationTimeStep;
	}

	FHitResult HitResult;
	const FVector TraceStart = FVector(PredictedLocation.X, PredictedLocation.Y, PredictedLocation.Z + StopLocationTraceHalfHeight);
	const FVector TraceEnd = FVector(PredictedLocation.X, PredictedLocation.Y, PredictedLocation.Z - StopLocationTraceHalfHeight);
	const EDrawDebugTrace::Type DrawDebugTrace = bDrawDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	const bool bHit = UKismetSystemLibrary::CapsuleTraceSingle(World, TraceStart, TraceEnd, CapsuleRadius, CapsuleHalfHeight, TraceChannel, false, ActorsToIgnore, DrawDebugTrace, HitResult, true, FLinearColor::Red, FLinearColor::Green, TraceDrawTime);

	if (bHit)
	{
		PredictResult.Location = FVector(HitResult.Location.X, HitResult.Location.Y, HitResult.Location.Z + DistanceToFloor);
		PredictResult.Time = PredictionTime;

		return;
	}

	PredictResult.Location = PredictedLocation;
	PredictResult.Time = PredictionTime;
}

void UDistanceMatchingComponent::PredictJumpPath(FPredictResult& PredictResult, const float SimulationTime, const float SimulationFrequency) const
{
	const float SubstepDeltaTime = 1.0f / SimulationFrequency;

	FVector CurrentVelocity = Velocity;
	FVector TraceStart = ActorLocation;
	FVector TraceEnd = TraceStart;
	float CurrentTime = 0.0f;

	while (CurrentTime < SimulationTime)
	{
		// Limit step to not go further than total time.
		const float PreviousTime = CurrentTime;
		const float ActualStepDeltaTime = FMath::Min(SimulationTime - CurrentTime, SubstepDeltaTime);
		CurrentTime += ActualStepDeltaTime;

		// Integrate (Velocity Verlet method)
		TraceStart = TraceEnd;
		FVector PreviousVelocity = CurrentVelocity;
		CurrentVelocity = PreviousVelocity + FVector(0.f, 0.f, GravityZ * ActualStepDeltaTime);
		TraceEnd = TraceStart + (PreviousVelocity + CurrentVelocity) * (0.5f * ActualStepDeltaTime);

		FHitResult HitResult;
		const EDrawDebugTrace::Type DrawDebugTrace = bDrawDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

		const bool bHit = UKismetSystemLibrary::CapsuleTraceSingle(World, TraceStart, TraceEnd, CapsuleRadius, CapsuleHalfHeight, TraceChannel, false, ActorsToIgnore, DrawDebugTrace, HitResult, true, FLinearColor::Red, FLinearColor::Green, TraceDrawTime);

		if (bHit)
		{
			PredictResult.Location = HitResult.Location;
			PredictResult.Time = PreviousTime + ActualStepDeltaTime * HitResult.Time;

			return;
		}
	}

	PredictResult.Location = TraceEnd;
	PredictResult.Time = SimulationTime;
}

void UDistanceMatchingComponent::PredictJumpApex(FPredictResult& PredictResult) const
{
	// Velocity * Sin jump angle / Gravity
	const float MaxTimeToApex = VelocitySize * Velocity.GetSafeNormal().Z / FMath::Abs(GravityZ);

	PredictJumpPath(PredictResult, MaxTimeToApex, ApexSimulationFrequency);
}

void UDistanceMatchingComponent::PredictLandingLocation(FPredictResult& PredictResult) const
{
	PredictJumpPath(PredictResult, MaxSimulationTime, LandingSimulationFrequency);

	PredictResult.Location += FVector(0.0f, 0.0f, DistanceToFloor);
}
