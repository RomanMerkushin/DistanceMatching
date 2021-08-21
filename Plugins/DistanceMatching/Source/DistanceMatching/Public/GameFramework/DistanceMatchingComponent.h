// Copyright Roman Merkushin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DistanceMatchingComponent.generated.h"

class UDistanceMatchingComponent;
class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovingSignature, UDistanceMatchingComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoppingSignature, UDistanceMatchingComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPivotingSignature, UDistanceMatchingComponent*, Component);

UENUM()
enum class EDistanceMatchingType : uint8
{
	Start,
	Stop,
	Pivot,
	None,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DISTANCEMATCHING_API UDistanceMatchingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDistanceMatchingComponent();

protected:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> Character;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	UPROPERTY(BlueprintReadOnly, Category="DistanceMatching")
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly, Category="DistanceMatching")
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category="DistanceMatching")
	float DistanceToMarker;

	UPROPERTY(BlueprintReadOnly, Category="DistanceMatching")
	EDistanceMatchingType DistanceMatchingType;

private:
	FVector ActorLocation;
	FVector PreviousActorLocation;
	FVector Velocity;
	FVector Acceleration;
	float PreviousAccelerationSize;
	FVector MarkerLocation;

public:
	/** Maximum simulation time for the stop location prediction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceMatching", meta=(ClampMin="0.1", ClampMax="5.0", UIMin="0.1", UIMax="5.0"))
	float MaxSimulationTime;

	/** Clamping the DistanceToMarker value between +-MaxDistanceToMarker to prevent float overflow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DistanceMatching")
	float MaxDistanceToMarker;

	UPROPERTY(BlueprintAssignable, Category="DistanceMatching")
	FOnMovingSignature OnMoving;

	UPROPERTY(BlueprintAssignable, Category="DistanceMatching")
	FOnStoppingSignature OnStopping;

	UPROPERTY(BlueprintAssignable, Category="DistanceMatching")
	FOnPivotingSignature OnPivoting;

private:
	/** Returns the predicted stop location for the character. */
	FVector PredictStopLocation(float DeltaTime) const;

public:
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool IsMoving() const { return bIsMoving; }
	bool IsAccelerating() const { return bIsAccelerating; }
	float GetDistanceToMarker() const { return DistanceToMarker; }
};
