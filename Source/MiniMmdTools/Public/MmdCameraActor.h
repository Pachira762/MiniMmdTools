// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "MmdCameraActor.generated.h"

/**
 * 
 */
UCLASS(Category = "MiniMmdTools")
class MINIMMDTOOLS_API AMmdCameraActor : public ACameraActor
{
	GENERATED_BODY()

public:
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UMmdCameraSequence> CameraSequence = nullptr;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	float Frame = 0.0;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	FVector LookAtOffset = FVector();

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	FVector LookAtScale = FVector(1.0, 1.0, 1.0);

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	FRotator RotationOffset = FRotator();

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	float DistanceScale = 1.f;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	float FieldOfViewScale = 1.f;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	float Zoom = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OutputFrameRate = 30.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseTemporalSampling = false;

	AMmdCameraActor(const FObjectInitializer& ObjectInitializer);

	virtual void	Tick(float DeltaTime) override;

	virtual bool	ShouldTickIfViewportsOnly() const override
	{
		return true;
	}

private:
	class UCineCameraComponent* CineCameraComponent;

	void UpdateCamera();
};
