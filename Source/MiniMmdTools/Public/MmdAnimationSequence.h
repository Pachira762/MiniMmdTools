// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MmdCommon.h"
#include "MmdAnimationSequence.generated.h"

USTRUCT(BlueprintType, Category = "MiniMmdTools")
struct MINIMMDTOOLS_API FMmdBoneKey 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 Frame = 0;

	UPROPERTY(EditAnywhere)
	FVector Location = FVector();

	UPROPERTY(EditAnywhere)
	FQuat Rotation = FQuat();

	UPROPERTY(EditAnywhere)
	FMmdInterpolation LocationXInterpolation;
	FMmdInterpolation LocationYInterpolation;
	FMmdInterpolation LocationZInterpolation;
	FMmdInterpolation RotationInterpolation;
};

USTRUCT(BlueprintType, Category = "MiniMmdTools")
struct MINIMMDTOOLS_API FMmdBoneTrack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString Name;

	UPROPERTY(EditAnywhere)
	TArray<FMmdBoneKey> Keys;
};

USTRUCT(BlueprintType, Category = "MiniMmdTools")
struct MINIMMDTOOLS_API FMmdMorphKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 Frame = 0;

	UPROPERTY(EditAnywhere)
	float Value = 0;
};

USTRUCT(BlueprintType, Category = "MiniMmdTools")
struct MINIMMDTOOLS_API FMmdMorphTrack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString Name;

	UPROPERTY(EditAnywhere)
	TArray<FMmdMorphKey> Keys;
};

UCLASS(BlueprintType, Category = "MiniMmdTools")
class MINIMMDTOOLS_API UMmdAnimationSequence : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FMmdBoneTrack> BoneTracks;

	UPROPERTY(EditAnywhere)
	TArray<FMmdMorphTrack> MorphTracks;
};
