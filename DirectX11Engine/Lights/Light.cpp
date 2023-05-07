#include "Core/pch.h"
#include "Light.h"

DirectionalLightData DirectionalLight::GetLightData()
{
	DirectionalLightData LightData;
	LightData.AmbientColor = AmbientColor;
	LightData.DiffuseColor = DiffuseColor;
	LightData.SpecularColor = SpecularColor;
	LightData.Direction = Direction;

	return LightData;
}

PointLightData PointLight::GetLightData()
{
	PointLightData LightData;
	LightData.AmbientColor = AmbientColor;
	LightData.DiffuseColor = DiffuseColor;
	LightData.SpecularColor = SpecularColor;
	LightData.Position = Position;
	LightData.Attenuation = Attenuation;
	LightData.Range = Range;

	return LightData;
}
