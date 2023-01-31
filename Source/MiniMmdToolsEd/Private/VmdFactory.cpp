// Fill out your copyright notice in the Description page of Project Settings.


#include "VmdFactory.h"
#include "MmdCameraSequence.h"
#include "MmdMorphSequence.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

#include "poml.h"

namespace
{
	using FInterpolation = poml::Interpolation;
	using FVmd = poml::VmdBase<FVector3f, FVector4f>;

	FName ShiftJisToName(const std::string& Str)
	{
		int Length = MultiByteToWideChar(CP_ACP, 0, Str.c_str(), -1, nullptr, 0);

		TArray<wchar_t> Buff;
		Buff.SetNum(Length + 1);
		MultiByteToWideChar(CP_ACP, 0, Str.c_str(), -1, Buff.GetData(), Buff.Num());

		return FName(Buff.GetData());
	}

	FMmdMorphKey ConvertMorphKey(const FVmd::MorphKey& InKey)
	{
		FMmdMorphKey OutKey;
		OutKey.Frame = static_cast<int32>(InKey.frame);
		OutKey.Value = InKey.value;

		return OutKey;
	}

	FMmdInterpolation ConvertInterpolation(const FInterpolation& InInterpolation)
	{
		FMmdInterpolation OutInterpolation;
		OutInterpolation.X1 = static_cast<float>(InInterpolation.x1) / 127.f;
		OutInterpolation.X2 = static_cast<float>(InInterpolation.x2) / 127.f;
		OutInterpolation.Y1 = static_cast<float>(InInterpolation.y1) / 127.f;
		OutInterpolation.Y2 = static_cast<float>(InInterpolation.y2) / 127.f;

		return OutInterpolation;
	}

	FMmdCameraKey ConvertCameraKey(const FVmd::CameraKey& InKey)
	{
		FMmdCameraKey OutKey;
		OutKey.Frame = InKey.frame;
		OutKey.Cut = 0;
		OutKey.Location = 8.0 * FVector(InKey.position.X, -InKey.position.Z, InKey.position.Y); // change axis

		double Pitch = FMath::RadiansToDegrees(InKey.rotation.X);
		double Yaw = FMath::RadiansToDegrees(-InKey.rotation.Y) - 90.0;
		double Roll = FMath::RadiansToDegrees(InKey.rotation.Z);
		OutKey.Rotation = FRotator(Pitch, Yaw, Roll);

		OutKey.Distance = -8.f * InKey.distance;
		OutKey.FieldOfView = static_cast<float>(InKey.view_angle);

		OutKey.InterpLocationX = ConvertInterpolation(InKey.ix);
		OutKey.InterpLocationY = ConvertInterpolation(InKey.iz); // change axis
		OutKey.InterpLocationZ = ConvertInterpolation(InKey.iy); // change axis
		OutKey.InterpRotation = ConvertInterpolation(InKey.ir);
		OutKey.InterpDistance = ConvertInterpolation(InKey.id);
		OutKey.InterpFieldOfView = ConvertInterpolation(InKey.iv);

		return OutKey;
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
	return (Class == UMmdCameraSequence::StaticClass() || Class == UMmdMorphSequence::StaticClass());
}

UObject* UVmdFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
	UObject* ImportedObject = nullptr;

	poml::VmdBase<FVector3f, FVector4f> Vmd;

	if (!poml::import_vmd(Buffer, BufferEnd - Buffer, Vmd))
	{
		return nullptr;
	}

	bool HasMorph = !Vmd.morph_tracks.empty();
	bool HasCamera = !Vmd.camera_track.empty();
	bool AddSuffix = HasMorph && HasCamera;

	if (HasMorph)
	{
		FName Name = AddSuffix ? FName(InName.ToString() + TEXT("_Morph")) : InName;
		UMmdMorphSequence* Sequence = ImportMorphSequence(Vmd, InParent, Name, Flags);

		if (!ImportedObject)
		{
			ImportedObject = Sequence;
		}
		else
		{
			AdditionalImportedObjects.Add(Sequence);
		}
	}

	if (HasCamera)
	{
		FName Name = AddSuffix ? FName(InName.ToString() + TEXT("_Camera")) : InName;
		UMmdCameraSequence* Sequence = ImportCameraSequence(Vmd, InParent, Name, Flags);

		if (!ImportedObject)
		{
			ImportedObject = Sequence;
		}
		else
		{
			AdditionalImportedObjects.Add(Sequence);
		}
	}

	return ImportedObject;
}

UMmdMorphSequence* UVmdFactory::ImportMorphSequence(const FVmd& Vmd, UObject* InParent, FName InName, EObjectFlags Flags)
{
	UMmdMorphSequence* Sequence = NewObject<UMmdMorphSequence>(InParent, InName, Flags);

	for (auto& [Name, VmdTrack] : Vmd.morph_tracks)
	{
		FMmdMorphTrack& Track = Sequence->Tracks.Add(ShiftJisToName(Name));

		for (auto& Key : VmdTrack)
		{
			Track.Keys.Add(ConvertMorphKey(Key));
		}
	}

	return Sequence;
}

UMmdCameraSequence* UVmdFactory::ImportCameraSequence(const FVmd& Vmd, UObject* InParent, FName InName, EObjectFlags Flags)
{
	UMmdCameraSequence* Sequence = NewObject<UMmdCameraSequence>(InParent, InName, Flags);

	Sequence->Keys.Reserve(static_cast<int32>(Vmd.camera_track.size()));

	for (auto& Key : Vmd.camera_track)
	{
		Sequence->Keys.Add(ConvertCameraKey(Key));
	}

	for (int i = 1; i < Sequence->Keys.Num(); ++i)
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
