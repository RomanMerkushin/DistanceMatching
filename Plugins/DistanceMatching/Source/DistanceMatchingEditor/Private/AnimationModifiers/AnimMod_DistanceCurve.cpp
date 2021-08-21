// Copyright Roman Merkushin. All Rights Reserved.

#include "AnimationModifiers/AnimMod_DistanceCurve.h"
#include "Animation/AnimSequence.h"
#include "AnimationBlueprintLibrary.h"

void UAnimMod_DistanceCurve::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	if (!AnimationSequence)
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

	const FVector StartLocation = GetRootBoneLocationAtFrame(AnimationSequence, 0);
	const FVector EndLocation = GetRootBoneLocationAtFrame(AnimationSequence, NumFrames);

	for (int32 Frame = 0; Frame <= NumFrames; Frame++)
	{
		float CurrentTime;
		UAnimationBlueprintLibrary::GetTimeAtFrame(AnimationSequence, Frame, CurrentTime);
		FVector CurrentLocation = GetRootBoneLocationAtFrame(AnimationSequence, Frame);

		const float Distance = bIsStartAnimation ? FVector::Distance(StartLocation, CurrentLocation) : FVector::Distance(CurrentLocation, EndLocation) * -1.0f;

		UAnimationBlueprintLibrary::AddFloatCurveKey(AnimationSequence, CurveName, CurrentTime, Distance);
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

FVector UAnimMod_DistanceCurve::GetRootBoneLocationAtFrame(const UAnimSequence* AnimationSequence, const int32 Frame) const
{
	FTransform Pose;
	UAnimationBlueprintLibrary::GetBonePoseForFrame(AnimationSequence, RootBoneName, Frame, true, Pose);

	return Pose.GetLocation();
}
