// Copyright Roman Merkushin. All Rights Reserved.

#include "Animation/AnimNode_DistanceMatching.h"
#include "Log.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimCurveCompressionCodec_UniformIndexable.h"

TAutoConsoleVariable<int32> CVarAnimNodeDistanceMatchingEnable(TEXT("a.AnimNode.DistanceMatching.Enable"), 1, TEXT("Toggle Distance Matching"));

float FAnimNode_DistanceMatching::GetCurrentAssetTime()
{
	return InternalTimeAccumulator;
}

float FAnimNode_DistanceMatching::GetCurrentAssetLength()
{
	return Sequence ? Sequence->GetPlayLength() : 0.0f;
}

void FAnimNode_DistanceMatching::OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	FAnimNode_AssetPlayerBase::OnInitializeAnimInstance(InProxy, InAnimInstance);

	PrevSequence = Sequence;

	// Caching CurveBuffer
	UpdateCurveBuffer();
}

void FAnimNode_DistanceMatching::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_AssetPlayerBase::Initialize_AnyThread(Context);
	GetEvaluateGraphExposedInputs().Execute(Context);

	InternalTimeAccumulator = 0.0f;

	// Update CurveBuffer if sequence is changed
	if (Sequence && Sequence != PrevSequence)
	{
		PrevSequence = Sequence;
		UpdateCurveBuffer();
	}
}

void FAnimNode_DistanceMatching::Evaluate_AnyThread(FPoseContext& Output)
{
	if (Sequence != nullptr && Output.AnimInstanceProxy->IsSkeletonCompatible(Sequence->GetSkeleton()))
	{
		FAnimationPoseData AnimationPoseData(Output);
		Sequence->GetAnimationPose(AnimationPoseData, FAnimExtractContext(InternalTimeAccumulator, Output.AnimInstanceProxy->ShouldExtractRootMotion()));
	}
	else
	{
		Output.ResetToRefPose();
	}
}

void FAnimNode_DistanceMatching::OverrideAsset(UAnimationAsset* NewAsset)
{
	if (UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(NewAsset))
	{
		Sequence = AnimSequence;
	}
}

void FAnimNode_DistanceMatching::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);

	if (Sequence != nullptr && Context.AnimInstanceProxy->IsSkeletonCompatible(Sequence->GetSkeleton()))
	{
		const bool bIsEnabled = CVarAnimNodeDistanceMatchingEnable.GetValueOnAnyThread() == 1;
		if (bIsEnabled)
		{
			if (!CurveBuffer.IsValid())
			{
				UE_LOG(LogDistanceMatching, Error, TEXT("CurveBuffer is nullptr"));
				return;
			}

			if (bEnableDistanceLimit && Distance >= DistanceLimit)
			{
				PlaySequence(Context);
			}
			else
			{
				InternalTimeAccumulator = FMath::Clamp(GetCurveTime(), 0.f, Sequence->GetPlayLength());
			}
		}
		else
		{
			PlaySequence(Context);
		}
	}
}

void FAnimNode_DistanceMatching::UpdateCurveBuffer()
{
	// Get curve SmartName
	FSmartName CurveSmartName;
	Sequence->GetSkeleton()->GetSmartNameByName(USkeleton::AnimCurveMappingName, DistanceCurveName, CurveSmartName);

	if (!CurveSmartName.IsValid())
	{
		UE_LOG(LogDistanceMatching, Error, TEXT("Can't retrieve curve smart name for %s"), *DistanceCurveName.ToString());
		return;
	}

	// Create a buffered access to times and values in curve
	CurveBuffer = MakeShareable(new FAnimCurveBufferAccess(Sequence, CurveSmartName.UID));
	if (!CurveBuffer->IsValid())
	{
		UE_LOG(LogDistanceMatching, Error, TEXT("Can't access to curve buffer by smart name: %s"), *CurveSmartName.DisplayName.ToString());
		return;
	}

	CurveBufferNumSamples = CurveBuffer->GetNumSamples();
}

float FAnimNode_DistanceMatching::GetCurveTime() const
{
	if (CurveBufferNumSamples == 0)
	{
		// If no keys in curve, return 0
		return 0.0f;
	}

	if (CurveBufferNumSamples < 2)
	{
		return CurveBuffer->GetTime(0);
	}

	if (Distance < CurveBuffer->GetValue(CurveBufferNumSamples - 1))
	{
		// Perform a lower bound to get the second of the interpolation nodes
		int32 First = 1;
		const int32 Last = CurveBufferNumSamples - 1;
		int32 Count = Last - First;

		while (Count > 0)
		{
			const int32 Step = Count / 2;
			const int32 Middle = First + Step;

			if (Distance >= CurveBuffer->GetValue(Middle))
			{
				First = Middle + 1;
				Count -= Step + 1;
			}
			else
			{
				Count = Step;
			}
		}

		const float Diff = CurveBuffer->GetValue(First) - CurveBuffer->GetValue(First - 1);

		if (Diff > 0.0f)
		{
			const float Alpha = (Distance - CurveBuffer->GetValue(First - 1)) / Diff;
			const float P0 = CurveBuffer->GetTime(First - 1);
			const float P3 = CurveBuffer->GetTime(First);

			// Find time by two nearest known points on the curve
			return FMath::Lerp(P0, P3, Alpha);
		}

		return CurveBuffer->GetTime(First - 1);
	}

	return CurveBuffer->GetTime(CurveBufferNumSamples - 1);
}

void FAnimNode_DistanceMatching::PlaySequence(const FAnimationUpdateContext& Context)
{
	InternalTimeAccumulator = FMath::Clamp(InternalTimeAccumulator, 0.f, Sequence->GetPlayLength());
	CreateTickRecordForNode(Context, Sequence, false, 1.0f);
}
