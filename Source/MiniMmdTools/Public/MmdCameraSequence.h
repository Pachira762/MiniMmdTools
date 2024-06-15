// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MmdCommon.h"
#include "MmdCameraSequence.generated.h"

USTRUCT(Category = "MiniMmdTools")
struct MINIMMDTOOLS_API FMmdCameraKey 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 Frame;

	UPROPERTY(EditAnywhere)
	int32 Cut;

	UPROPERTY(EditAnywhere)
	FVector Location;

	UPROPERTY(EditAnywhere)
	FRotator Rotation;

	UPROPERTY(EditAnywhere)
	float Distance;

	UPROPERTY(EditAnywhere)
	float FieldOfView;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation LocationXInterpolation;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation LocationYInterpolation;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation LocationZInterpolation;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation RotationInterpolation;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation DistanceInterpolation;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation FieldOfViewInterpolation;
};

UCLASS(Category = "MiniMmdTools")
class MINIMMDTOOLS_API UMmdCameraSequence : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FMmdCameraKey> Keys;

	UPROPERTY(EditAnywhere)
	int32 FrameOffset;

	void CalcCameraProperty(int32 Frame, float Subframe, int32& OutCutNo, FVector& OutLocation, FRotator& OutRotation, float& OutDistance, float& OutFieldOfView) const;
};
