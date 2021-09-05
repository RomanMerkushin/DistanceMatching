// Copyright Roman Merkushin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_AssetPlayerBase.h"
#include "Animation/AnimNode_DistanceMatching.h"
#include "AnimGraphNode_DistanceMatching.generated.h"

UCLASS(MinimalAPI)
class UAnimGraphNode_DistanceMatching : public UAnimGraphNode_AssetPlayerBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	FAnimNode_DistanceMatching Node;

private:
	FNodeTextCache CachedNodeTitle;

public:
	// UEdGraphNode interface
	virtual FLinearColor GetNodeTitleColor() const override { return FLinearColor::Black; }
	virtual FText GetTooltipText() const override { return CachedNodeTitle; }
	virtual FText GetNodeTitle(const ENodeTitleType::Type TitleType) const override;
	virtual FText GetMenuCategory() const override;
	// End of UEdGraphNode interface

	// UAnimGraphNode_Base interface
	virtual void ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog) override;
	virtual void PreloadRequiredAssets() override;
	virtual void BakeDataDuringCompilation(FCompilerResultsLog& MessageLog) override;
	virtual bool DoesSupportTimeForTransitionGetter() const override { return true; }
	virtual UAnimationAsset* GetAnimationAsset() const override;
	virtual const TCHAR* GetTimePropertyName() const override;
	virtual UScriptStruct* GetTimePropertyStruct() const override;
	virtual void GetAllAnimationSequencesReferred(TArray<UAnimationAsset*>& AnimationAssets) const override;
	virtual void ReplaceReferredAnimations(const TMap<UAnimationAsset*, UAnimationAsset*>& AnimAssetReplacementMap) override;
	virtual EAnimAssetHandlerType SupportsAssetClass(const UClass* AssetClass) const override;
	// End of UAnimGraphNode_Base interface

	// UAnimGraphNode_AssetPlayerBase interface
	virtual void SetAnimationAsset(UAnimationAsset* Asset) override;
	// End of UAnimGraphNode_AssetPlayerBase interface

private:
	void UpdateNodeTitleForSequence(const ENodeTitleType::Type TitleType, const UAnimSequenceBase* InSequence) const;
};
