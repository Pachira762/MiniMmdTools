// Fill out your copyright notice in the Description page of Project Settings.


#include "VmdFactory.h"
#include "MmdAnimationSequence.h"
#include "MmdCameraSequence.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

#include "poml.h"

namespace
{
	using FVmdVector3 = FVector3f;
	using FVmdVector4 = FVector4f;
	using FVmdInterpolation = poml::Interpolation;
	using FVmd = poml::VmdBase<FVmdVector3, FVmdVector4>;

	FString ShiftJisToString(const std::string& Str)
	{
		int Length = MultiByteToWideChar(CP_ACP, 0, Str.c_str(), -1, nullptr, 0);

		TArray<wchar_t> Buff;
		Buff.SetNum(Length + 1);
		MultiByteToWideChar(CP_ACP, 0, Str.c_str(), -1, Buff.GetData(), Buff.Num());

		return FString(Buff.GetData());
	}

	FVector ConvertLocation(const FVmdVector3& VmdLocation)
	{
		return 8.0 * FVector(VmdLocation.X, -VmdLocation.Z, VmdLocation.Y);
	}

	FQuat ConvertQuaternion(const FVmdVector4& VmdQuaternion)
	{
		return FQuat(VmdQuaternion.X, -VmdQuaternion.Z, VmdQuaternion.Y, VmdQuaternion.W);
	}

	FVector ConvertCameraEuler(const FVmdVector3& VmdRotation)
	{
		double Pitch = FMath::RadiansToDegrees(VmdRotation.X);
		double Yaw = FMath::RadiansToDegrees(-VmdRotation.Y) - 90.0;
		double Roll = FMath::RadiansToDegrees(VmdRotation.Z);
		return FVector(Pitch, Yaw, Roll);
	}

	FMmdInterpolation ConvertInterpolation(const FVmdInterpolation& VmdInterpolation)
	{
		float X1 = static_cast<float>(VmdInterpolation.x1) / 127.f;
		float Y1 = static_cast<float>(VmdInterpolation.y1) / 127.f;
		float X2 = static_cast<float>(VmdInterpolation.x2) / 127.f;
		float Y2 = static_cast<float>(VmdInterpolation.y2) / 127.f;
		return FMmdInterpolation{ X1, Y1, X2, Y2 };
	}
}

UVmdFactory::UVmdFactory()
	: Super()
{
	bCreateNew = false;
	bEditorImport = true;
	bText = false;
	Formats.Add(TEXT("vmd;Vocaloid Motion Data File"));

	SupportedClass = UMmdCameraSequence::StaticClass();
}

bool UVmdFactory::DoesSupportClass(UClass* Class)
{
	return (Class == UMmdCameraSequence::StaticClass() || Class == UMmdAnimationSequence::StaticClass());
}

UObject* UVmdFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
	UObject* ImportedObject = nullptr;

	poml::VmdBase<FVector3f, FVector4f> Vmd;
	if (!poml::import_vmd(Buffer, BufferEnd - Buffer, Vmd))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to import VMD file."));
		return nullptr;
	}

	bool bHasAnimation = !Vmd.motion_tracks.empty() || !Vmd.morph_tracks.empty();
	bool bHasCamera = !Vmd.camera_track.empty();

	if (bHasAnimation)
	{
		FName Name = InName;
		UMmdAnimationSequence* AnimationSequence = ImportAnimaionSequence(Vmd, InParent, Name, Flags);

		if (!ImportedObject)
		{
			ImportedObject = AnimationSequence;
		}
		else
		{
			AdditionalImportedObjects.Add(AnimationSequence);
		}
	}

	if (bHasCamera)
	{
		FName Name = bHasAnimation ? FName(InName.ToString() + TEXT("_Camera")) : InName;
		UMmdCameraSequence* CameraSequence = ImportCameraSequence(Vmd, InParent, Name, Flags);

		if (!ImportedObject)
		{
			ImportedObject = CameraSequence;
		}
		else
		{
			AdditionalImportedObjects.Add(CameraSequence);
		}
	}

	return ImportedObject;
}

UMmdAnimationSequence* UVmdFactory::ImportAnimaionSequence(const FVmd& Vmd, UObject* InParent, FName InName, EObjectFlags Flags)
{
	UMmdAnimationSequence* Sequence = NewObject<UMmdAnimationSequence>(InParent, InName, Flags);

	for (auto& [Name, VmdKeys] : Vmd.motion_tracks)
	{
		FMmdBoneTrack Track;
		Track.Name = ShiftJisToString(Name);

		for (auto& VmdKey : VmdKeys)
		{
			FMmdBoneKey Key;
			Key.Frame = VmdKey.frame;
			Key.Location = ConvertLocation(VmdKey.position);
			Key.Rotation = ConvertQuaternion(VmdKey.orientation);
			Key.LocationXInterpolation = ConvertInterpolation(VmdKey.ix);
			Key.LocationYInterpolation = ConvertInterpolation(VmdKey.iz); // change axis
			Key.LocationZInterpolation = ConvertInterpolation(VmdKey.iy); // change axis
			Key.RotationInterpolation = ConvertInterpolation(VmdKey.ir);

			Track.Keys.Add(Key);
		}

		Sequence->BoneTracks.Add(Track);
	}

	for (auto& [Name, VmdKeys] : Vmd.morph_tracks)
	{
		FMmdMorphTrack Track;
		Track.Name = ShiftJisToString(Name);

		for (auto& VmdKey : VmdKeys)
		{
			FMmdMorphKey Key;
			Key.Frame = VmdKey.frame;
			Key.Value = VmdKey.value;

			Track.Keys.Add(Key);
		}

		Sequence->MorphTracks.Add(Track);
	}

	return Sequence;
}

UMmdCameraSequence* UVmdFactory::ImportCameraSequence(const FVmd& Vmd, UObject* InParent, FName InName, EObjectFlags Flags)
{
	UMmdCameraSequence* Sequence = NewObject<UMmdCameraSequence>(InParent, InName, Flags);

	for (auto& VmdKey : Vmd.camera_track)
	{
		FMmdCameraKey Key;
		Key.Frame = VmdKey.frame;
		Key.Cut = 0;
		Key.Location = ConvertLocation(VmdKey.position);
		Key.Rotation = ConvertCameraEuler(VmdKey.rotation);
		Key.Distance = -8.f * VmdKey.distance;
		Key.FieldOfView = static_cast<float>(VmdKey.view_angle);
		Key.LocationXInterpolation = ConvertInterpolation(VmdKey.ix);
		Key.LocationYInterpolation = ConvertInterpolation(VmdKey.iz); // change axis
		Key.LocationZInterpolation = ConvertInterpolation(VmdKey.iy); // change axis
		Key.RotationInterpolation = ConvertInterpolation(VmdKey.ir);
		Key.DistanceInterpolation = ConvertInterpolation(VmdKey.id);
		Key.FieldOfViewInterpolation = ConvertInterpolation(VmdKey.iv);

		Sequence->Keys.Add(Key);
	}

	for (int32 i = 1, NumKeys = Sequence->Keys.Num(); i < NumKeys; ++i)
	{
		FMmdCameraKey& Key = Sequence->Keys[i];
		const FMmdCameraKey& Prev = Sequence->Keys[i - 1];

		if (Key.Frame == Prev.Frame + 1)
		{
			Key.Cut = Prev.Cut + 1;
		}
		else
		{
			Key.Cut = Prev.Cut;
		}
	}

	return Sequence;
}
