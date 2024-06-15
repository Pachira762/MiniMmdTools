// Fill out your copyright notice in the Description page of Project Settings.


#include "MmdCameraActor.h"
#include "CineCameraComponent.h"
#include "MmdCameraSequence.h"

namespace
{
	TPair<int32, float> CalcFrameNumber(float Frame, float FrameRate, bool bTemporalSampling)
	{
		if (bTemporalSampling)
		{
			float Sec = Frame / 30.f;
			float OutputFrame = FMath::RoundToFloat(Sec * FrameRate);
			int32 SequenceFrame = static_cast<int32>(OutputFrame / FrameRate * 30.f); // 30FPS
			float Subframe = (30.f * Sec - SequenceFrame) + 0.5f;

			return { SequenceFrame, Subframe };
		}
		else
		{
			return { FMath::RoundToInt32(Frame), 0.f };
		}
	}

	float ToHorizontalFoV(float VerticalAngle, float SensorWidth, float SensorHeight)
	{
		float AngleV = FMath::DegreesToRadians(VerticalAngle / 2.f);
		float Length = SensorHeight / FMath::Tan(AngleV);
		float AngleH = FMath::Atan2(SensorWidth, Length);
		return 2.f * FMath::RadiansToDegrees(AngleH);
	}
}

AMmdCameraActor::AMmdCameraActor(const FObjectInitializer& ObjectInitialzier)
	: Super(ObjectInitialzier.SetDefaultSubobjectClass<UCineCameraComponent>(TEXT("CameraComponent")))
{
	CineCameraComponent = Cast<UCineCameraComponent>(GetCameraComponent());

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AMmdCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CameraSequence && !CameraSequence->Keys.IsEmpty())
	{
		UpdateCamera();
	}
}

void AMmdCameraActor::UpdateCamera()
{
	auto [FrameNo, Subframe] = CalcFrameNumber(Frame, OutputFrameRate, bUseTemporalSampling);

	int32 CutNo;
	FVector LookAtLocation;
	FRotator Rotation;
	float Distance;
	float FieldOfView;
	CameraSequence->CalcCameraProperty(FrameNo, Subframe, CutNo, LookAtLocation, Rotation, Distance, FieldOfView);

	LookAtLocation = LookAtScale * LookAtLocation + LookAtOffset;
	Rotation += RotationOffset;
	Distance *= DistanceScale * Zoom;
	FVector Location = LookAtLocation - Distance * Rotation.Quaternion().GetForwardVector();

	const FCameraFilmbackSettings& Filmback = CineCameraComponent->Filmback;
	FieldOfView = ToHorizontalFoV(FieldOfView * FieldOfViewScale / Zoom, Filmback.SensorWidth, Filmback.SensorHeight);

	CineCameraComponent->SetRelativeLocationAndRotation(Location, Rotation);
	CineCameraComponent->SetFieldOfView(FieldOfView);
}
