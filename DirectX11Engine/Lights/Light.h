#pragma once
#include "Core/pch.h"
#include "Core/Actor.h"

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
class Light : public Actor
{
public:

	Light(DirectX::XMFLOAT3 NewPosition, DirectX::XMFLOAT4 NewAmbientColor, DirectX::XMFLOAT4 NewDiffuseColor, DirectX::XMFLOAT4 NewSpecularColor);

	// Position of the light in the world
	//DirectX::XMFLOAT3 Position;

	// Light's colors
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT4 SpecularColor;
};

// Directional light, with a direction
class DirectionalLight : public Light
{
public:

	DirectionalLight(DirectX::XMFLOAT3 NewPosition, DirectX::XMFLOAT4 NewAmbientColor,	DirectX::XMFLOAT4 NewDiffuseColor,	DirectX::XMFLOAT4 NewSpecularColor, DirectX::XMFLOAT3 NewRotation);

	// Convert this light to data for use in shaders
	DirectionalLightData GetLightData();

	//DirectX::XMFLOAT3 Direction{};
};


// Base point light class
class PointLight : public Light
{
public:

	PointLight(DirectX::XMFLOAT3 NewPosition, DirectX::XMFLOAT4 NewAmbientColor, DirectX::XMFLOAT4 NewDiffuseColor, DirectX::XMFLOAT4 NewSpecularColor, DirectX::XMFLOAT3 NewAttenuation);

	// Convert this light to data for use in shaders
	PointLightData GetLightData();

	DirectX::XMFLOAT3 Attenuation;
	float Range = 500;
};