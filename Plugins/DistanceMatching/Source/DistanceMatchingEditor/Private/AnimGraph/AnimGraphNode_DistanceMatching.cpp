// Copyright Roman Merkushin. All Rights Reserved.

#include "AnimGraph/AnimGraphNode_DistanceMatching.h"
#include "EditorCategoryUtils.h"
#include "Animation/AnimComposite.h"
#include "Kismet2/CompilerResultsLog.h"

#define LOCTEXT_NAMESPACE "AnimGraphNode_DistanceMatching"

FLinearColor UAnimGraphNode_DistanceMatching::GetNodeTitleColor() const
{
	return FLinearColor::Black;
}

FText UAnimGraphNode_DistanceMatching::GetTooltipText() const
{
	return CachedNodeTitle;
}

FText UAnimGraphNode_DistanceMatching::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	if (Node.Sequence == nullptr)
	{
		// We may have a valid variable connected or default pin value
		UEdGraphPin* SequencePin = FindPin(GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_DistanceMatching, Sequence));
		if (SequencePin && SequencePin->LinkedTo.Num() > 0)
		{
			CachedNodeTitle.SetCachedText(LOCTEXT("DistanceMatching_TitleVariable", "Distance Matching"), this);
		}
		else if (SequencePin && SequencePin->DefaultObject != nullptr)
		{
			GetNodeTitleForSequence(TitleType, CastChecked<UAnimSequenceBase>(SequencePin->DefaultObject));
		}
		else
		{
			CachedNodeTitle.SetCachedText(LOCTEXT("DistanceMatching_TitleNONE", "Distance Matching (None)"), this);
		}
	}
	else
	{
		GetNodeTitleForSequence(TitleType, Node.Sequence);
	}

	return CachedNodeTitle;
}

FText UAnimGraphNode_DistanceMatching::GetMenuCategory() const
{
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Animation);
}

void UAnimGraphNode_DistanceMatching::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);

	UAnimSequenceBase* SequenceToCheck = Node.Sequence;
	UEdGraphPin* SequencePin = FindPin(GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_DistanceMatching, Sequence));
	if (SequencePin != nullptr && SequenceToCheck == nullptr)
	{
		SequenceToCheck = Cast<UAnimSequenceBase>(SequencePin->DefaultObject);
	}

	if (SequenceToCheck == nullptr)
	{
		// Check for bindings
		bool bHasBinding = false;
		if (SequencePin != nullptr)
		{
			if (PropertyBindings.Find(SequencePin->GetFName()))
			{
				bHasBinding = true;
			}
		}

		// We may have a connected node or binding
		if (SequencePin == nullptr || SequencePin->LinkedTo.Num() == 0 && !bHasBinding)
		{
			MessageLog.Error(TEXT("@@ references an unknown sequence"), this);
		}
	}
	else if (SupportsAssetClass(SequenceToCheck->GetClass()) == EAnimAssetHandlerType::NotSupported)
	{
		const FText SequenceDisplayName = SequenceToCheck->GetClass()->GetDisplayNameText();
		const FText ErrorMessage = FText::Format(
			LOCTEXT("UnsupportedAssetError", "@@ is trying to play a {0} as a sequence, which is not allowed."), SequenceDisplayName);
		MessageLog.Error(*ErrorMessage.ToString(), this);
	}
	else
	{
		USkeleton* SeqSkeleton = SequenceToCheck->GetSkeleton();
		// If anim sequence doesn't have skeleton, it might be due to anim sequence not loaded yet
		if (SeqSkeleton && !SeqSkeleton->IsCompatible(ForSkeleton))
		{
			MessageLog.Error(TEXT("@@ references sequence that uses different skeleton @@"), this, SeqSkeleton);
		}
	}
}

void UAnimGraphNode_DistanceMatching::BakeDataDuringCompilation(FCompilerResultsLog& MessageLog)
{
	UAnimBlueprint* AnimBlueprint = GetAnimBlueprint();
	AnimBlueprint->FindOrAddGroup(SyncGroup.GroupName);
	Node.GroupName = SyncGroup.GroupName;
	Node.GroupRole = SyncGroup.GroupRole;
	Node.Method = SyncGroup.Method;
}

UAnimationAsset* UAnimGraphNode_DistanceMatching::GetAnimationAsset() const
{
	UAnimSequenceBase* Sequence = Node.Sequence;
	UEdGraphPin* SequencePin = FindPin(GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_DistanceMatching, Sequence));
	if (SequencePin != nullptr && Sequence == nullptr)
	{
		Sequence = Cast<UAnimSequenceBase>(SequencePin->DefaultObject);
	}

	return Sequence;
}

const TCHAR* UAnimGraphNode_DistanceMatching::GetTimePropertyName() const
{
	return TEXT("InternalTimeAccumulator");
}

UScriptStruct* UAnimGraphNode_DistanceMatching::GetTimePropertyStruct() const
{
	return FAnimNode_DistanceMatching::StaticStruct();
}

EAnimAssetHandlerType UAnimGraphNode_DistanceMatching::SupportsAssetClass(const UClass* AssetClass) const
{
	if (AssetClass->IsChildOf(UAnimSequence::StaticClass()) || AssetClass->IsChildOf(UAnimComposite::StaticClass()))
	{
		return EAnimAssetHandlerType::PrimaryHandler;
	}

	return EAnimAssetHandlerType::NotSupported;
}

void UAnimGraphNode_DistanceMatching::SetAnimationAsset(UAnimationAsset* Asset)
{
	if (UAnimSequenceBase* Seq = Cast<UAnimSequenceBase>(Asset))
	{
		Node.Sequence = Seq;
	}
}

FText UAnimGraphNode_DistanceMatching::GetNodeTitleForSequence(ENodeTitleType::Type TitleType, UAnimSequenceBase* InSequence) const
{
	const FText SequenceName = FText::FromString(InSequence->GetName());

	FFormatNamedArguments Args;
	Args.Add(TEXT("SequenceName"), SequenceName);
	CachedNodeTitle.SetCachedText(FText::Format(LOCTEXT("DistanceMatching", "Distance Matching: {SequenceName}"), Args), this);

	return CachedNodeTitle;
}

#undef LOCTEXT_NAMESPACE
