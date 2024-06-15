// Copyright Epic Games, Inc. All Rights Reserved.

#include "MmdCommon.h"
#include "poml.h"

float FMmdInterpolation::AlphaAt(float Time) const
{
	return poml::calc_bezier<8>(Time, X1, X2, Y1, Y2);
}
