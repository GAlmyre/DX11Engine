#include "Core/pch.h"
#include "Camera.h"
#include "Core/Math.h"

Camera::Camera()
{
	UpdateViewMatrix();
}

Camera::Camera(XMVECTOR Position, XMVECTOR Target, XMVECTOR UpVector)
{
	SetPosition(Position);
	SetTarget(Target);
	SetUpVector(UpVector);

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	ViewMatrix = XMMatrixLookToLH(Position, ForwardVector, UpVector);
}

void Camera::UpdateProjectionMatrix(int Width, int Height)
{
	ProjectionMatrix = XMMatrixPerspectiveFovLH(Math::DegreesToRadian(80.0f), Width / Height, 0.1f, 10000.0f);
}

DirectX::XMVECTOR Camera::GetForwardVector()
{
	return ForwardVector;
}

DirectX::XMVECTOR Camera::GetRightVector()
{
	return -XMVector3Cross(GetForwardVector(), GetUpVector());
}
