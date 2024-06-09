#include "Math.h"

float Math::RadianToDegrees(float RadValue)
{
	float Res = RadValue * (180 / PI);
	return UnwindDegrees(Res);
}

float Math::DegreesToRadian(float DegValue)
{
	float Res = DegValue * (PI / 180);

	return Res;
}

float Math::UnwindDegrees(float A)
{
	while (A > 180.f)
	{
		A -= 360.f;
	}

	while (A < -180.f)
	{
		A += 360.f;
	}

	return A;
}
