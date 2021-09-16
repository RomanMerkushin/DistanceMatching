// Copyright Roman Merkushin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "GameFramework/DistanceMatchingTypes.h"
#include "AnimMod_DistanceCurve.generated.h"

UCLASS()
class DISTANCEMATCHINGEDITOR_API UAnimMod_DistanceCurve : public UAnimationModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	FName RootBoneName;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FName CurveName;

	UPROPERTY(EditAnywhere, Category = "Settings")
	EDistanceMatchingType DistanceMatchingType;

	UAnimMod_DistanceCurve();

	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
	virtual void OnRevert_Implementation(UAnimSequence* AnimationSequence) override;

private:
	/** Returns location for the root bone at the specified Frame from the given Animation Sequence. */
	FVector GetRootBoneLocationAtFrame(const TObjectPtr<UAnimSequence> AnimationSequence, const int32 Frame) const;

	/** Returns the frame index with zero distance. */
	int32 GetStartIndex(const TObjectPtr<UAnimSequence> AnimationSequence, const int32 NumFrames) const;

	/** Sets a distance values in the float curve inside of the given Animation Sequence. */
	void SetDistanceCurveKeys(const TObjectPtr<UAnimSequence> AnimationSequence, const int32 StartIndex, const int32 EndIndex, const bool bRevert) const;
};
