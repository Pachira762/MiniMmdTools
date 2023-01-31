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
	TObjectPtr<class UMmdCameraSequence>	CameraSequence;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	float	Frame;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	FVector LookAtOffset;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	FVector LookAtScale = FVector(1.0, 1.0, 1.0);

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	FRotator RotationOffset;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	float	DistanceScale = 1.f;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	float	FieldOfViewScale = 1.f;

	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite)
	float	Zoom = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32	OutputFrameRate = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseTemporalSampling = false;

	AMmdCameraActor(const FObjectInitializer& ObjectInitializer);

	virtual void	Tick(float DeltaTime) override;

	virtual bool	ShouldTickIfViewportsOnly() const override;

private:
	class UCineCameraComponent* CineCameraComponent;

	void UpdateCamera();

	TPair<int32, float> CalcFrame() const;

	float ToHorizontalFoV(float VerticalFoV) const;
};
