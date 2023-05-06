#pragma once
#include "pch.h"

// The base class for all lights

struct DirectionalLightData
{
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT4 SpecularColor;
	DirectX::XMFLOAT3 Direction;
	float PadDir;
};

struct PointLightData
{
	DirectX::XMFLOAT3 Position;
	float PadPos;
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT4 SpecularColor;
	DirectX::XMFLOAT3 Attenuation;
	float Range;
};

// Base class for all lights
class Light
{
public:

	// Position of the light in the world
	DirectX::XMFLOAT3 Position;

	// Light's colors
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT4 SpecularColor;
};

// Directional light, with a direction
class DirectionalLight : public Light
{
public:

	DirectX::XMFLOAT3 Direction;

	// Convert this light to data for use in shaders
	DirectionalLightData GetLightData();
};


// Base point light class
class PointLight : public Light
{
public:

	DirectX::XMFLOAT3 Attenuation;
	float Range;

	// Convert this light to data for use in shaders
	PointLightData GetLightData();
};