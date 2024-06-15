// Fill out your copyright notice in the Description page of Project Settings.


#include "MmdCameraSequence.h"
#include "MmdCommon.h"

void UMmdCameraSequence::CalcCameraProperty(int32 Frame, float Subframe, int32& OutCutNo, FVector& OutLocation, FRotator& OutRotation, float& OutDistance, float& OutFieldOfView) const
{
	if (Keys.IsEmpty()) 
	{
		OutLocation = FVector::ZeroVector;
		OutRotation = FRotator(0.0, 0.0, 0.0);
		OutDistance = 0.0;
		OutFieldOfView = 90.0;
		return;
	}

	auto [Key0, Key1] = FindKeyByFrame(Keys, Frame);
	if (Key0.Cut != Key1.Cut || Key0.Frame == Key1.Frame)
	{
		OutCutNo = Key0.Cut;
		OutLocation = Key0.Location;
		OutRotation = Key0.Rotation;
		OutDistance = Key0.Distance;
		OutFieldOfView = Key0.FieldOfView;
	}
	else
	{
		const float Time = (Frame - Key0.Frame + Subframe) / (Key1.Frame - Key0.Frame);
		OutCutNo = Key0.Cut;
		OutLocation.X = Key1.LocationXInterpolation.Interp(Key0.Location.X, Key1.Location.X, Time);
		OutLocation.Y = Key1.LocationYInterpolation.Interp(Key0.Location.Y, Key1.Location.Y, Time);
		OutLocation.Z = Key1.LocationZInterpolation.Interp(Key0.Location.Z, Key1.Location.Z, Time);
		OutRotation = Key1.RotationInterpolation.Interp(Key0.Rotation, Key1.Rotation, Time);
		OutDistance = Key1.DistanceInterpolation.Interp(Key0.Distance, Key1.Distance, Time);
		OutFieldOfView = Key1.FieldOfViewInterpolation.Interp(Key0.FieldOfView, Key1.FieldOfView, Time);
	}
}
