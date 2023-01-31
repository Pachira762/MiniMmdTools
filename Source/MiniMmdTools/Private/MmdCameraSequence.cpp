// Fill out your copyright notice in the Description page of Project Settings.


#include "MmdCameraSequence.h"
#include "poml.h"

namespace
{
	float CalcNormalizedTime(const FMmdCameraKey& Key0, const FMmdCameraKey& Key1, int32 Frame, float Subframe)
	{
		if (Key0.Frame == Key1.Frame)
		{
			return 0.f;
		}

		float Time = (static_cast<float>(Frame - Key0.Frame) + Subframe) / static_cast<float>(Key1.Frame - Key0.Frame);

		return FMath::Clamp(Time, 0.f, 1.f);
	}
}

float FMmdInterpolation::CalcValue(float X) const
{
	return poml::calc_bezier<8>(X, X1, X2, Y1, Y2);
}

FMmdCameraProperty UMmdCameraSequence::CalcCameraProperty(int32 Frame, float Subframe) const
{
	if (Keys.IsEmpty()) 
	{
		return FMmdCameraProperty{};
	}

	const auto[Index0, Index1] = SearchKeyIndex(Frame, Subframe);
	const FMmdCameraKey& Key0 = Keys[Index0];
	const FMmdCameraKey& Key1 = Keys[Index1];
	
	const float Time = CalcNormalizedTime(Key0, Key1, Frame, Subframe);

	float AlphaLocationX = Key1.InterpLocationX.CalcValue(Time);
	float AlphaLocationY = Key1.InterpLocationY.CalcValue(Time);
	float AlphaLocationZ = Key1.InterpLocationZ.CalcValue(Time);
	float AlphaRotation = Key1.InterpRotation.CalcValue(Time);
	float AlphaDistance = Key1.InterpDistance.CalcValue(Time);
	float AlphaFieldOfView = Key1.InterpFieldOfView.CalcValue(Time);

	FMmdCameraProperty Props{};
	Props.Location.X = FMath::Lerp(Key0.Location.X, Key1.Location.X, AlphaLocationX);
	Props.Location.Y = FMath::Lerp(Key0.Location.Y, Key1.Location.Y, AlphaLocationY);
	Props.Location.Z = FMath::Lerp(Key0.Location.Z, Key1.Location.Z, AlphaLocationZ);
	Props.Rotation.Pitch = FMath::Lerp(Key0.Rotation.Pitch, Key1.Rotation.Pitch, AlphaRotation);
	Props.Rotation.Yaw = FMath::Lerp(Key0.Rotation.Yaw, Key1.Rotation.Yaw, AlphaRotation);
	Props.Rotation.Roll = FMath::Lerp(Key0.Rotation.Roll, Key1.Rotation.Roll, AlphaRotation);
	Props.Distance = FMath::Lerp(Key0.Distance, Key1.Distance, AlphaDistance);
	Props.FieldOfView = FMath::Lerp(Key0.FieldOfView, Key1.FieldOfView, AlphaFieldOfView);

	return Props;
}

TPair<int32, int32> UMmdCameraSequence::SearchKeyIndex(int32 Frame, float Subframe) const
{
	int32 Index1 = Algo::UpperBoundBy(Keys, Frame, &FMmdCameraKey::Frame);
	int32 Index0 = Index1 - 1;

	Index0 = FMath::Clamp(Index0, 0, Keys.Num() - 1);
	Index1 = FMath::Clamp(Index1, 0, Keys.Num() - 1);

	if (Keys[Index0].Cut != Keys[Index1].Cut)
	{
		Index1 = Index0;
	}

	return { Index0, Index1 };
}
