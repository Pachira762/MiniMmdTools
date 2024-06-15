// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "poml.h"
#include "VmdFactory.generated.h"

class UMmdAnimationSequence;
class UMmdCameraSequence;

/**
 * 
 */
UCLASS()
class MINIMMDTOOLSED_API UVmdFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVmdFactory();

	virtual bool DoesSupportClass(UClass* Class)override;

	virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)override;

private:
	using FVmd = poml::VmdBase<FVector3f, FVector4f>;

	UMmdAnimationSequence* ImportAnimaionSequence(const FVmd& Vmd, UObject* InParent, FName InName, EObjectFlags Flags);

	UMmdCameraSequence* ImportCameraSequence(const FVmd& Vmd, UObject* InParent, FName InName, EObjectFlags Flags);
};
