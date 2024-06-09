#include "Math.h"

float Math::RadianToDegrees(float RadValue)
{
	float Res = RadValue * (180 / PI);
	return Res;
}

float Math::DegreesToRadian(float DegValue)
{
	float Res = DegValue * (PI / 180);

	return Res;
}
