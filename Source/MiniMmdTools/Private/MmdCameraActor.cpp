// Fill out your copyright notice in the Description page of Project Settings.


#include "MmdCameraActor.h"
#include "CineCameraComponent.h"
#include "MmdCameraSequence.h"

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

bool AMmdCameraActor::ShouldTickIfViewportsOnly() const
{
	return true;
}

void AMmdCameraActor::UpdateCamera()
{
	auto [FrameNo, Subframe] = CalcFrame();
	FMmdCameraProperty Props = CameraSequence->CalcCameraProperty(FrameNo, Subframe);

	FVector LookAtLocation = LookAtScale * Props.Location + LookAtOffset;
	FRotator Rotation = Props.Rotation + RotationOffset;
	float Distance = Props.Distance * DistanceScale * Zoom;
	FVector Location = LookAtLocation - Distance * Rotation.Quaternion().GetForwardVector();

	float FieldOfView = ToHorizontalFoV(Props.FieldOfView * FieldOfViewScale / Zoom);

	CineCameraComponent->SetRelativeLocationAndRotation(Location, Rotation);
	CineCameraComponent->SetFieldOfView(FieldOfView);
}

TPair<int32, float> AMmdCameraActor::CalcFrame() const
{
	if (bUseTemporalSampling)
	{
		float Sec = Frame / 30.f;
		float OutputFrame = FMath::RoundToFloat(Sec * OutputFrameRate);
		int32 SequenceFrame = static_cast<int32>(OutputFrame * (30.f / OutputFrameRate));
		float Subframe = 30.f * (Sec - SequenceFrame / 30.f) + 0.5;

		return { SequenceFrame, Subframe };
	}
	else
	{
		return { FMath::RoundToInt32(Frame), 0.f };
	}
}

float AMmdCameraActor::ToHorizontalFoV(float VerticalFoV) const
{
	const FCameraFilmbackSettings& Filmback = CineCameraComponent->Filmback;

	float AngleV = FMath::DegreesToRadians(VerticalFoV / 2.f);
	float Length = Filmback.SensorHeight / FMath::Tan(AngleV);
	float AngleH = FMath::Atan2(Filmback.SensorWidth, Length);

	return 2.f * FMath::RadiansToDegrees(AngleH);
}
