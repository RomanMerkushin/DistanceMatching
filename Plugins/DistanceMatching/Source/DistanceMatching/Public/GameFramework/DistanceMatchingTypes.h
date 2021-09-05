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

USTRUCT()
struct FPredictResult
{
	GENERATED_BODY()

	/** Predicted marker location. */
	UPROPERTY()
	FVector Location;

	/** Time to marker location. */
	UPROPERTY()
	float Time;

	FPredictResult()
		: Location(ForceInitToZero)
		, Time(0.0f)
	{
	}
};
