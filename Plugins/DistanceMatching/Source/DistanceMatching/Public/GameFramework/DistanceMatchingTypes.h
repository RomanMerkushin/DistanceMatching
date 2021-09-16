// Copyright Roman Merkushin. All Rights Reserved.

#pragma once

#include "DistanceMatchingTypes.generated.h"

UENUM(BlueprintType)
enum class EDistanceMatchingType : uint8
{
	Start,
	Stop,
	Pivot,
	Jump,
	Fall,
	None,
};

USTRUCT(BlueprintType)
struct FPredictResult
{
	GENERATED_BODY()

	/** Predicted marker location. */
	UPROPERTY(BlueprintReadOnly)
	FVector Location;

	/** Distance to marker location. */
	UPROPERTY(BlueprintReadOnly)
	float Distance;

	/** Time to marker location. */
	UPROPERTY(BlueprintReadOnly)
	float Time;

	FPredictResult()
		: Location(ForceInitToZero)
		, Distance(0.0f)
		, Time(0.0f)
	{
	}
};
