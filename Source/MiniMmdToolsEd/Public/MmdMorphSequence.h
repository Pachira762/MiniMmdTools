// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MmdMorphSequence.generated.h"

USTRUCT()
struct FMmdMorphKey
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere)
	int32 Frame;

	UPROPERTY(EditAnywhere)
	float Value;
};

USTRUCT()
struct FMmdMorphTrack
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere)
	TArray<FMmdMorphKey> Keys;
};

/**
 * 
 */
UCLASS(BlueprintType, Category = "MiniMmdTools")
class MINIMMDTOOLSED_API UMmdMorphSequence : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TMap<FName, FMmdMorphTrack> Tracks;

	UFUNCTION(BlueprintCallable, Category = "MiniMmdTools")
	void ApplyToAnimSequence(UAnimSequence* AnimSequence, int32 FrameOffset = 0, bool bOnlyCompatibleMorph = true) const;
};
