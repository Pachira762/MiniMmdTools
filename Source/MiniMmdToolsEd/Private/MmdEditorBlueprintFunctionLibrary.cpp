// Fill out your copyright notice in the Description page of Project Settings.


#include "MmdEditorBlueprintFunctionLibrary.h"
#include "MmdAnimationSequence.h"

void UMmdEditorBlueprintFunctionLibrary::ApplyMorph(UAnimSequence* AnimSequence, const UMmdAnimationSequence* MmdAnimationSequence, int32 FrameOffset)
{
	if (!AnimSequence || !MmdAnimationSequence)
	{
		return;
	}

	USkeletalMesh* Mesh = AnimSequence->GetPreviewMesh();
	if (Mesh == nullptr)
	{
		const USkeleton* Skeleton = AnimSequence->GetSkeleton();
		check(Skeleton != nullptr);

		Mesh = Skeleton->GetPreviewMesh();
	}

	check(Mesh != nullptr);

	IAnimationDataController& Controller = AnimSequence->GetController();
	check(Controller.GetModel() != nullptr);

	for (auto& Track : MmdAnimationSequence->MorphTracks)
	{
		FName Name = FName(Track.Name);

		if (!Mesh->FindMorphTarget(Name))
		{
			continue;
		}

		FAnimationCurveIdentifier CurveId = FAnimationCurveIdentifier(Name, ERawCurveTrackTypes::RCT_Float);
		if (!Controller.GetModel()->FindCurve(CurveId))
		{
			if (!Controller.AddCurve(CurveId))
			{
				continue;
			}
		}
		
		TArray<FRichCurveKey> CurveKeys;
		for (auto& Key : Track.Keys)
		{
			float Time = static_cast<float>(Key.Frame + FrameOffset) / 30.f;
			CurveKeys.Add(FRichCurveKey(Time, Key.Value));
		}

		Controller.SetCurveKeys(CurveId, CurveKeys);
	}
}
