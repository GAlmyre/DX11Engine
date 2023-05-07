#pragma once
#include "Core/pch.h"

struct MaterialData
{
	// Ambient Color
	DirectX::XMFLOAT3 AmbientColor = DirectX::XMFLOAT3(1,1,1);
	float PadAmbient;
	// Diffuse Color
	DirectX::XMFLOAT3 DiffuseColor = DirectX::XMFLOAT3(.3, .2, .2);
	float PadDiffuse;
	// Specular Color
	DirectX::XMFLOAT3 SpecularColor = DirectX::XMFLOAT3(1, 0, 0);
	// Specular Exponent
	float SpecExp = 64.0f;
};
