#include "Actor.h"
#include "Math.h"

void Actor::UpdateWorldMatrix()
{
	DirectX::XMMATRIX TranslationMatrix = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);
	float RotationX = Math::DegreesToRadian(Rotation.x);
	float RotationY = Math::DegreesToRadian(Rotation.y);
	float RotationZ = Math::DegreesToRadian(Rotation.z);
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(RotationX, RotationY, RotationZ);
	DirectX::XMMATRIX ScaleMatrix = DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);

	WorldMatrix = ScaleMatrix * RotationMatrix * TranslationMatrix;
}

DirectX::XMFLOAT3 Actor::GetForwardVector() const
{
	DirectX::XMFLOAT3 DefaultForward = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	DirectX::XMFLOAT3 RotationRad = DirectX::XMFLOAT3(Math::DegreesToRadian(Rotation.x), Math::DegreesToRadian(Rotation.y), Math::DegreesToRadian(Rotation.z));
	DirectX::XMVECTOR ForwardVector = DirectX::XMVector3Transform(XMLoadFloat3(&DefaultForward), DirectX::XMMatrixRotationRollPitchYaw(RotationRad.x, RotationRad.y, RotationRad.z));
	DirectX::XMFLOAT3 Result;
	XMStoreFloat3(&Result, ForwardVector);

	return Result;
}

DirectX::XMFLOAT3 Actor::GetRightVector() const
{
	DirectX::XMFLOAT3 DefaultRight = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 RotationRad = DirectX::XMFLOAT3(Math::DegreesToRadian(Rotation.x), Math::DegreesToRadian(Rotation.y), Math::DegreesToRadian(Rotation.z));
	DirectX::XMVECTOR RightVector = DirectX::XMVector3Transform(XMLoadFloat3(&DefaultRight), DirectX::XMMatrixRotationRollPitchYaw(RotationRad.x, RotationRad.y, RotationRad.z));
	DirectX::XMFLOAT3 Result;
	XMStoreFloat3(&Result, RightVector);

	return Result;
}

DirectX::XMFLOAT3 Actor::GetUpVector() const
{
	DirectX::XMFLOAT3 DefaultUp = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	DirectX::XMFLOAT3 RotationRad = DirectX::XMFLOAT3(Math::DegreesToRadian(Rotation.x), Math::DegreesToRadian(Rotation.y), Math::DegreesToRadian(Rotation.z));
	DirectX::XMVECTOR UpVector = DirectX::XMVector3Transform(XMLoadFloat3(&DefaultUp), DirectX::XMMatrixRotationRollPitchYaw(RotationRad.x, RotationRad.y, RotationRad.z));
	DirectX::XMFLOAT3 Result;
	XMStoreFloat3(&Result, UpVector);

	return Result;
}