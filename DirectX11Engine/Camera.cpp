#include "pch.h"
#include "Camera.h"

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
	ProjectionMatrix = XMMatrixPerspectiveFovLH(0.4f * 3.14f, Width / Height, 0.1f, 10000.0f);
}

DirectX::XMVECTOR Camera::GetForwardVector()
{
	return ForwardVector;
}

DirectX::XMVECTOR Camera::GetRightVector()
{
	return -XMVector3Cross(GetForwardVector(), GetUpVector());
}
