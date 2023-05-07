#pragma once
#include "Light.h"
class PointLight :
	public Light
{
	DirectX::XMFLOAT3 Attenuation;
	float Range;
};

