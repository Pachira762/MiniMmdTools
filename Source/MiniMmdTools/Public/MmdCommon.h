// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MmdCommon.generated.h"

USTRUCT(Category = "MiniMmdTools")
struct MINIMMDTOOLS_API FMmdInterpolation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float X1;

	UPROPERTY(EditAnywhere)
	float Y1;

	UPROPERTY(EditAnywhere)
	float X2;

	UPROPERTY(EditAnywhere)
	float Y2;

	float AlphaAt(float Time) const;

	template<typename T>
	T Interp(const T& A, const T& B, float Time) const
	{
		float Alpha = AlphaAt(Time);
		return FMath::Lerp(A, B, Alpha);
	}

	template<>
	FQuat Interp(const FQuat& A, const FQuat& B, float Time) const
	{
		float Alpha = AlphaAt(Time);
		return FQuat::Slerp(A, B, Alpha);
	}
};

template<typename T>
TPair<const T&, const T&> FindKeyByFrame(const TArray<T>& Keys, int32 Frame)
{
	int32 Index1 = Algo::UpperBoundBy(Keys, Frame, &T::Frame);
	int32 Index0 = Index1 - 1;

	return {
		Keys[FMath::Clamp(Index0, 0, Keys.Num() - 1)],
		Keys[FMath::Clamp(Index1, 0, Keys.Num() - 1)],
	};
}
