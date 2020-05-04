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
	ViewMatrix = XMMatrixLookAtLH(Position, Target, UpVector);
}

void Camera::UpdateProjectionMatrix(float Width, float Height)
{
	ProjectionMatrix = XMMatrixPerspectiveFovLH(0.4f * 3.14f, Width / Height, 1.0f, 1000.0f);
}

DirectX::XMVECTOR Camera::GetForwardVector()
{
	return XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
}

DirectX::XMVECTOR Camera::GetRightVector()
{
	return XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
}
