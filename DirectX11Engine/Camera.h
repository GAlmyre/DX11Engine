#pragma once
#include "pch.h"

using namespace DirectX;

class Camera
{
public:

	Camera();

	Camera(XMVECTOR Position, XMVECTOR Target, XMVECTOR UpVector);

	// Getters and Setters for vectors and matrices
	XMVECTOR GetTarget() const { return Target; }
	void SetTarget(XMVECTOR NewTarget)
	{ 
		Target = NewTarget;
		UpdateViewMatrix();
	}

	XMVECTOR GetPosition() const { return Position; }
	void SetPosition(XMVECTOR NewPosition) 
	{ 
		Position = NewPosition;
		UpdateViewMatrix();
	}

	XMVECTOR GetUpVector() const { return UpVector; }
	void SetUpVector(XMVECTOR NewUp) 
	{ 
		UpVector = NewUp;
		UpdateViewMatrix();
	}

	XMMATRIX GetViewMatrix() const { return ViewMatrix; }
	void SetViewMatrix(XMMATRIX NewViewMatrix) { ViewMatrix = NewViewMatrix; }

	XMMATRIX GetProjectionMatrix() const { return ProjectionMatrix; }
	void SetProjectionMatrix(XMMATRIX NewProjectionMatrix) { ProjectionMatrix = NewProjectionMatrix; }

	// Updates the ViewMatrix depending on Position, Target and Up
	void UpdateViewMatrix();

	void UpdateProjectionMatrix(int Width, int Height);
	
	XMVECTOR GetForwardVector();
	void SetForwardVector(XMVECTOR NewForward)
	{
		ForwardVector = NewForward;
		UpdateViewMatrix();
	}
	XMVECTOR GetRightVector();

	float Speed = .5;

private:
	// Camera coordinates
	XMVECTOR Position = { 0.0f, 0.0f, -5.0f, 0.0f };
	// Pitch, yaw and roll
	XMFLOAT3 Rotation = { 0.0f, 0.0f, 0.0f };

	XMVECTOR Target = { 0.0f, 0.0f, 0.0f, 0.0f };

	XMVECTOR ForwardVector = { 0.0f, 0.0f, 1.0f, 0.0f };
	XMVECTOR UpVector = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;
};

