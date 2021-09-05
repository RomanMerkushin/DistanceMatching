// Copyright Roman Merkushin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
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
	bool bIsStartAnimation;

	UAnimMod_DistanceCurve();

	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
	virtual void OnRevert_Implementation(UAnimSequence* AnimationSequence) override;

private:
	FVector GetRootBoneLocationAtFrame(const UAnimSequence* AnimationSequence, const int32 Frame) const;
};
