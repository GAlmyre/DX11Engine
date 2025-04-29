#pragma once
#include "Core/pch.h"

struct MaterialData
{
	// Ambient Color
	DirectX::XMFLOAT4 AmbientColor = DirectX::XMFLOAT4(1, 1, 1, 1);
	// Diffuse Color
	DirectX::XMFLOAT4 DiffuseColor = DirectX::XMFLOAT4(.3, .2, .2, 1);
	// Specular Color
	DirectX::XMFLOAT4 SpecularColor = DirectX::XMFLOAT4(1, 0, 0, 1);
	// Specular Exponent
	float SpecExp = 32.0f;

	int bUseAlbedoTexture = 0;
	int bUseNormalMap = 0;
	int bUseSpecularMap = 0;
};
