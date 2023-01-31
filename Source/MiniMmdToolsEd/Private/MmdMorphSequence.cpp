// Fill out your copyright notice in the Description page of Project Settings.


#include "MmdMorphSequence.h"
#include "AnimationBlueprintLibrary.h"

namespace
{
	bool HasMorphTarget(USkeletalMesh* Mesh, const FName& Name)
	{
		if(!Mesh)
		{
			return true;
		}
		
		return Mesh->FindMorphTarget(Name) != nullptr;
	}
}

void UMmdMorphSequence::ApplyToAnimSequence(UAnimSequence* AnimSequence, int32 FrameOffset, bool bOnlyCompatibleMorph) const
{
	USkeletalMesh* Mesh = AnimSequence->GetPreviewMesh();

	for (auto& [Name, Track] : Tracks)
	{
		if (bOnlyCompatibleMorph && !HasMorphTarget(Mesh, Name))
		{
			continue;
		}

		if (!UAnimationBlueprintLibrary::DoesCurveExist(AnimSequence, Name, ERawCurveTrackTypes::RCT_Float)) 
		{
			UAnimationBlueprintLibrary::AddCurve(AnimSequence, Name);
		}

		const FName ContainerName = UAnimationBlueprintLibrary::RetrieveContainerNameForCurve(AnimSequence, Name);
		if (ContainerName == NAME_None)
		{
			continue;
		}

		const FSmartName CurveSmartName = UAnimationBlueprintLibrary::RetrieveSmartNameForCurve(AnimSequence, Name, ContainerName);
		const FAnimationCurveIdentifier CurveId(CurveSmartName, ERawCurveTrackTypes::RCT_Float);
		
		TArray<FRichCurveKey> CurveKeys;
		for (auto& [Frame, Value] : Track.Keys)
		{
			CurveKeys.Add(FRichCurveKey(static_cast<float>(Frame + FrameOffset) / 30.f, Value));
		}

		CurveKeys.Sort([](const FRichCurveKey& Key0, const FRichCurveKey& Key1) -> bool
			{
				return Key0.Time < Key1.Time;
			});

		AnimSequence->GetController().SetCurveKeys(CurveId, CurveKeys, false);
	}
}