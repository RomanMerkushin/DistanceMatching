// Copyright Roman Merkushin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "AnimNode_DistanceMatching.generated.h"

struct FAnimCurveBufferAccess;

USTRUCT(BlueprintInternalUseOnly)
struct DISTANCEMATCHING_API FAnimNode_DistanceMatching : public FAnimNode_AssetPlayerBase
{
	GENERATED_BODY()

	FAnimNode_DistanceMatching();

	// FAnimNode_AssetPlayerBase interface
	virtual float GetCurrentAssetTime() override { return InternalTimeAccumulator; }
	virtual float GetCurrentAssetLength() override;
	virtual UAnimationAsset* GetAnimAsset() override { return Sequence; }
	// End of FAnimNode_AssetPlayerBase interface

	// FAnimNode_Base interface
	virtual bool NeedsOnInitializeAnimInstance() const override { return true; }
	virtual void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void OverrideAsset(UAnimationAsset* NewAsset) override;
	virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;
	// End of FAnimNode_Base interface

private:
	TSharedPtr<FAnimCurveBufferAccess> CurveBuffer;
	int32 CurveBufferNumSamples;
	TObjectPtr<UAnimSequenceBase> PrevSequence;
	uint8 bIsEnabled : 1;

public:
	/** The animation sequence asset to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (PinShownByDefault, DisallowedClasses = "AnimMontage"))
	TObjectPtr<UAnimSequenceBase> Sequence;

	/** The distance value to search in curve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (PinShownByDefault))
	float Distance;

	/** The name of the distance curve in animation sequence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (PinHiddenByDefault))
	FName DistanceCurveName;

	/** Continue play animation as normal when distance limit is exceeded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (PinHiddenByDefault))
	bool bEnableDistanceLimit;

	/** Distance matching limit. See bEnableDistanceLimit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (PinHiddenByDefault, EditCondition = "bEnableDistanceLimit"))
	float DistanceLimit;

private:
	/** Update curve buffer from sequence by curve name. */
	void UpdateCurveBuffer();

	/** Returns the time of a named curve for corresponding distance value. */
	float GetCurveTime() const;

	/** Play animation sequence. */
	void PlaySequence(const FAnimationUpdateContext& Context);
};
