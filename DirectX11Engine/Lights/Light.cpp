#include "Core/pch.h"
#include "Light.h"
#include "Core/Math.h"

Light::Light(DirectX::XMFLOAT3 NewPosition, DirectX::XMFLOAT4 NewAmbientColor, DirectX::XMFLOAT4 NewDiffuseColor, DirectX::XMFLOAT4 NewSpecularColor)
	:AmbientColor(NewAmbientColor), DiffuseColor(NewDiffuseColor), SpecularColor(NewSpecularColor)
{
	SetPosition(NewPosition);
}

DirectX::XMMATRIX Light::GetViewMatrix() const
{
	// Compute the view matrix (used for shadow maps)

	DirectX::XMVECTOR PositionVect = DirectX::XMLoadFloat3(&Position);
	DirectX::XMFLOAT3 LookAt = DirectX::XMFLOAT3(0,0,0);
	DirectX::XMVECTOR LookAtVect = DirectX::XMLoadFloat3(&LookAt);

	DirectX::XMFLOAT3 LookTo = GetForwardVector();
	DirectX::XMVECTOR LookToVect = DirectX::XMLoadFloat3(&LookTo);

	DirectX::XMFLOAT3 Up = GetUpVector();
	DirectX::XMVECTOR UpVect = DirectX::XMLoadFloat3(&Up);

	//return DirectX::XMMatrixLookAtLH(PositionVect, LookAtVect, UpVect);
	return DirectX::XMMatrixLookToLH(PositionVect, LookToVect, UpVect);
}

DirectX::XMMATRIX Light::GetProjectionMatrix(float Width, float Height) const
{
	return DirectX::XMMatrixOrthographicLH(Width, Height, 5.0f, 1000.0f);
	//return DirectX::XMMatrixPerspectiveFovLH(PI/2.0f, 1.0f/*Width / Height*/, 0.1f, 5000.0f);
	//return DirectX::XMMatrixPerspectiveFovLH(3.14159265358979323846f / 2.0f, 1.0f, 0.1f, 1000.0f);
}

DirectionalLight::DirectionalLight(DirectX::XMFLOAT3 NewPosition, DirectX::XMFLOAT4 NewAmbientColor, DirectX::XMFLOAT4 NewDiffuseColor, DirectX::XMFLOAT4 NewSpecularColor, DirectX::XMFLOAT3 NewRotation)
:Light(NewPosition, NewAmbientColor, NewDiffuseColor, NewSpecularColor)
{
	SetRotation(NewRotation);
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
	LightData.Direction = GetForwardVector();

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