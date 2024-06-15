// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MmdEditorBlueprintFunctionLibrary.generated.h"

class UAnimSequence;
class UMmdAnimationSequence;

UCLASS(BlueprintType, Category = "MiniMmdTools")
class MINIMMDTOOLSED_API UMmdEditorBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "MiniMmdTools")
	static void ApplyMorph(UAnimSequence* AnimSequence, const UMmdAnimationSequence* MmdAnimationSequence, int32 FrameOffset = 0);
};
