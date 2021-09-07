// Copyright Roman Merkushin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DistanceMatchingTypes.h"
#include "DistanceMatchingComponent.generated.h"

// Max DistanceToMarker or TimeToMarker value to prevent float overflow.
#define MAX_MARKER_VALUE (1000.0f)
// Minimum value to determine if character moving or accelerating.
#define MOVEMENT_THRESHOLD (0.00001f)

class UCapsuleComponent;
class UCharacterMovementComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISTANCEMATCHING_API UDistanceMatchingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDistanceMatchingComponent();
	virtual void InitializeComponent() override;
	virtual void TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Variables from character and its components
	FVector ActorLocation;
	FVector PreviousActorLocation;
	FVector Velocity;
	float VelocitySize;
	FVector Acceleration;
	float AccelerationSize;
	float PreviousAccelerationSize;
	float CapsuleRadius;
	float CapsuleHalfHeight;
	float DistanceToFloor;
	float GravityZ;

	// Debug flags
	uint8 bShowDebug : 1;
	uint8 bDrawDebugTrace : 1;

	// Flags of character movement states
	uint8 bIsMoving : 1;
	uint8 bIsAccelerating : 1;
	uint8 bIsFalling : 1;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UWorld> World;

	UPROPERTY(Transient)
	TObjectPtr<ACharacter> Character;

	UPROPERTY(Transient)
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	EDistanceMatchingType DistanceMatchingType;
	FVector MarkerLocation;
	float DistanceToMarker;
	float TimeToMarker;

public:
	/** Maximum simulation time for the stop or pivot location prediction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceMatching", meta = (ClampMin = 0.1f, ClampMax = 5.0f, UIMin = 0.1f, UIMax = 5.0f))
	float MaxSimulationTime;

	/** Determines size of each sub-step in the simulation for apex prediction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceMatching", meta = (ClampMin = 1.0f, ClampMax = 30.0f, UIMin = 1.0f, UIMax = 30.0f))
	float ApexSimulationFrequency;

	/** Determines size of each sub-step in the simulation for landing prediction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceMatching", meta = (ClampMin = 1.0f, ClampMax = 30.0f, UIMin = 1.0f, UIMax = 30.0f))
	float LandingSimulationFrequency;

	/** Minimum angle for pivot detection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceMatching", meta = (ClampMin = 0.0f, ClampMax = 180.0f, UIMin = 0.0f, UIMax = 180.0f))
	float MinPivotAngle;

	/** Channel for all kind of traces used for distance matching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceMatching|Trace")
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	/** Actors which will be ignored for all kind of traces used for distance matching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceMatching|Trace")
	TArray<AActor*> ActorsToIgnore;

	/** Half height of capsule trace for Z value correction when predicting stop location on a slope. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceMatching|Trace", meta = (ClampMin = 100.0f, ClampMax = 1000.0f, UIMin = 100.0f, UIMax = 1000.0f))
	float StopLocationTraceHalfHeight;

	/** Debug sphere radius for markers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceMatching|Debug")
	float DebugSphereRadius;

	/** How long shows debug spheres when debug mode is on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceMatching|Debug", meta = (ClampMin = 0.1f, ClampMax = 10.0f, UIMin = 0.1f, UIMax = 10.0f))
	float DebugDrawTime;

	/** Debug draw time for all kind of traces used for distance matching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DistanceMatching|Debug", meta = (ClampMin = 0.1f, ClampMax = 10.0f, UIMin = 0.1f, UIMax = 10.0f))
	float TraceDrawTime;

private:
	/**
	* Predict the stop or pivot location for the character.
	*
	* @param PredictResult		Output result of the prediction (location and time).
	* @param DeltaTime			The time since the last tick.
	*/
	void PredictStopLocation(FPredictResult& PredictResult, const float DeltaTime) const;

	/**
	* Predict the arc of a jump path affected by gravity with collision checks along the arc.
	*
	* @param PredictResult			Output result of the prediction (location and time).
	* @param SimulationTime			Maximum simulation time for the jump path prediction.
	* @param SimulationFrequency	Determines size of each sub-step in the simulation (chopping up SimulationTime).
	*/
	void PredictJumpPath(FPredictResult& PredictResult, const float SimulationTime = 2.0f, const float SimulationFrequency = 10.0f) const;

	/** Predict the jump apex location and time to it. */
	void PredictJumpApex(FPredictResult& PredictResult) const;

	/** Predict the jump landing location and time to it. */
	void PredictLandingLocation(FPredictResult& PredictResult) const;

public:
	/** Returns true, if character speed is greater than zero. */
	UFUNCTION(BlueprintCallable, Category = "DistanceMatching")
	FORCEINLINE bool IsMoving() const { return bIsMoving; }

	/** Returns true, if character acceleration is greater than zero. */
	UFUNCTION(BlueprintCallable, Category = "DistanceMatching")
	FORCEINLINE bool IsAccelerating() const { return bIsAccelerating; }

	/** Returns true if currently falling (not flying, in a non-fluid volume, and not on the ground). */
	UFUNCTION(BlueprintCallable, Category = "DistanceMatching")
	FORCEINLINE bool IsFalling() const { return bIsFalling; }

	/** Returns current type of distance matching. */
	UFUNCTION(BlueprintCallable, Category = "DistanceMatching")
	FORCEINLINE EDistanceMatchingType GetDistanceMatchingType() const { return DistanceMatchingType; }

	/** Returns the distance remaining to marker location (can be positive in case of distance to start marker or negative in other cases). */
	UFUNCTION(BlueprintCallable, Category = "DistanceMatching")
	FORCEINLINE float GetDistanceToMarker() const { return DistanceToMarker; }

	/** Returns the time to reach the marker location. */
	UFUNCTION(BlueprintCallable, Category = "DistanceMatching")
	FORCEINLINE float GetTimeToMarker() const { return TimeToMarker; }
};
