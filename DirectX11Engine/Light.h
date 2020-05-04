#pragma once
#include "pch.h"

// The base class for all lights

class Light
{
public:

	DirectX::XMFLOAT3 Direction;
	float PadDir;

	DirectX::XMFLOAT3 Position;
	float Range;

	DirectX::XMFLOAT3 Attenuation;
	float PadAtt;

	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
};

