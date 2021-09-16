// Copyright Roman Merkushin. All Rights Reserved.

#include "AnimationModifiers/AnimMod_DistanceCurve.h"
#include "Animation/AnimSequence.h"
#include "AnimationBlueprintLibrary.h"

UAnimMod_DistanceCurve::UAnimMod_DistanceCurve()
	: RootBoneName(FName("root"))
	, CurveName(FName("Distance"))
	, DistanceMatchingType(EDistanceMatchingType::Stop)
{
}

void UAnimMod_DistanceCurve::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	if (!AnimationSequence || DistanceMatchingType == EDistanceMatchingType::None)
	{
		return;
	}

	const bool bCurveExists = UAnimationBlueprintLibrary::DoesCurveExist(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float);

	if (bCurveExists)
	{
		UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, CurveName, false);
	}
	UAnimationBlueprintLibrary::AddCurve(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float, false);

	int32 NumFrames;
	UAnimationBlueprintLibrary::GetNumFrames(AnimationSequence, NumFrames);

	const int32 StartIndex = GetStartIndex(AnimationSequence, NumFrames);

	if (DistanceMatchingType == EDistanceMatchingType::Start || DistanceMatchingType == EDistanceMatchingType::Pivot)
	{
		SetDistanceCurveKeys(AnimationSequence, StartIndex, NumFrames, false);
	}
	if (DistanceMatchingType == EDistanceMatchingType::Stop || DistanceMatchingType == EDistanceMatchingType::Pivot)
	{
		const float EndIndex = DistanceMatchingType == EDistanceMatchingType::Pivot ? StartIndex : NumFrames;
		SetDistanceCurveKeys(AnimationSequence, 0, EndIndex, true);
	}
}

void UAnimMod_DistanceCurve::OnRevert_Implementation(UAnimSequence* AnimationSequence)
{
	if (!AnimationSequence)
	{
		return;
	}

	UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, CurveName, false);
}

FVector UAnimMod_DistanceCurve::GetRootBoneLocationAtFrame(const TObjectPtr<UAnimSequence> AnimationSequence, const int32 Frame) const
{
	FTransform Pose;
	UAnimationBlueprintLibrary::GetBonePoseForFrame(AnimationSequence, RootBoneName, Frame, true, Pose);

	return Pose.GetLocation();
}

int32 UAnimMod_DistanceCurve::GetStartIndex(const TObjectPtr<UAnimSequence> AnimationSequence, const int32 NumFrames) const
{
	if (DistanceMatchingType != EDistanceMatchingType::Pivot)
	{
		return 0;
	}

	FVector PreviousLocation = GetRootBoneLocationAtFrame(AnimationSequence, 0);
	for (int32 Frame = 1; Frame <= NumFrames; Frame++)
	{
		FVector CurrentLocation = GetRootBoneLocationAtFrame(AnimationSequence, Frame);

		if (CurrentLocation.Size() < PreviousLocation.Size())
		{
			return Frame - 1;
		}

		PreviousLocation = CurrentLocation;
	}

	return 0;
}

void UAnimMod_DistanceCurve::SetDistanceCurveKeys(const TObjectPtr<UAnimSequence> AnimationSequence, const int32 StartIndex, const int32 EndIndex, const bool bRevert) const
{
	const FVector StartLocation = GetRootBoneLocationAtFrame(AnimationSequence, StartIndex);
	const FVector EndLocation = GetRootBoneLocationAtFrame(AnimationSequence, EndIndex);

	for (int32 Frame = StartIndex; Frame <= EndIndex; Frame++)
	{
		float CurrentTime;
		UAnimationBlueprintLibrary::GetTimeAtFrame(AnimationSequence, Frame, CurrentTime);
		FVector CurrentLocation = GetRootBoneLocationAtFrame(AnimationSequence, Frame);

		const float Distance = bRevert ? FVector::Distance(CurrentLocation, EndLocation) * -1.0f : FVector::Distance(StartLocation, CurrentLocation);

		UAnimationBlueprintLibrary::AddFloatCurveKey(AnimationSequence, CurveName, CurrentTime, Distance);
	}
}
