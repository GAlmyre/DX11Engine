#include "Core/pch.h"
#include "Light.h"

Light::Light(DirectX::XMFLOAT3 NewPosition, DirectX::XMFLOAT4 NewAmbientColor, DirectX::XMFLOAT4 NewDiffuseColor, DirectX::XMFLOAT4 NewSpecularColor)
	:AmbientColor(NewAmbientColor), DiffuseColor(NewDiffuseColor), SpecularColor(NewSpecularColor)
{
	SetPosition(NewPosition);
}

DirectionalLight::DirectionalLight(DirectX::XMFLOAT3 NewPosition, DirectX::XMFLOAT4 NewAmbientColor, DirectX::XMFLOAT4 NewDiffuseColor, DirectX::XMFLOAT4 NewSpecularColor, DirectX::XMFLOAT3 NewDirection)
:Light(NewPosition, NewAmbientColor, NewDiffuseColor, NewSpecularColor)
{
	SetRotation(NewDirection);
	UpdateWorldMatrix();
}

PointLight::PointLight(DirectX::XMFLOAT3 NewPosition, DirectX::XMFLOAT4 NewAmbientColor, DirectX::XMFLOAT4 NewDiffuseColor, DirectX::XMFLOAT4 NewSpecularColor, DirectX::XMFLOAT3 NewAttenuation)
	:Light(NewPosition, NewAmbientColor, NewDiffuseColor, NewSpecularColor), Attenuation(NewAttenuation)
{
	UpdateWorldMatrix();
}

DirectionalLightData DirectionalLight::GetLightData()
{
	DirectionalLightData LightData{};
	LightData.AmbientColor = AmbientColor;
	LightData.DiffuseColor = DiffuseColor;
	LightData.SpecularColor = SpecularColor;
	LightData.Direction = Rotation;

	return LightData;
}

PointLightData PointLight::GetLightData()
{
	PointLightData LightData{};
	LightData.AmbientColor = AmbientColor;
	LightData.DiffuseColor = DiffuseColor;
	LightData.SpecularColor = SpecularColor;
	LightData.Position = Position;
	LightData.Attenuation = Attenuation;
	LightData.Range = Range;

	return LightData;
}