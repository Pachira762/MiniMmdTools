// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MmdCameraSequence.generated.h"

USTRUCT(Category = "MiniMmdTools")
struct MINIMMDTOOLS_API FMmdInterpolation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float X1;

	UPROPERTY(EditAnywhere)
	float X2;

	UPROPERTY(EditAnywhere)
	float Y1;

	UPROPERTY(EditAnywhere)
	float Y2;

	float CalcValue(float Time) const;
};

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
	FMmdInterpolation InterpLocationX;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation InterpLocationY;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation InterpLocationZ;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation InterpRotation;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation InterpDistance;

	UPROPERTY(EditAnywhere)
	FMmdInterpolation InterpFieldOfView;
};

struct FMmdCameraProperty
{
	FVector Location;

	FRotator Rotation;

	float Distance;

	float FieldOfView;
};

/**
 * 
 */
UCLASS(Category = "MiniMmdTools")
class MINIMMDTOOLS_API UMmdCameraSequence : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FMmdCameraKey> Keys;

	UPROPERTY(EditAnywhere)
	int32 FrameOffset;

	FMmdCameraProperty CalcCameraProperty(int32 Frame, float Subframe) const;

private:
	TPair<int32, int32> SearchKeyIndex(int32 Frame, float Subframe) const;
};
