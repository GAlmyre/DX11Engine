#pragma once
#include "Core/pch.h"

class Actor
{
public:
	// World Matrix
	DirectX::XMMATRIX GetWorldMatrix() const { return WorldMatrix; }
	void SetWorldMatrix(DirectX::XMMATRIX Val) { WorldMatrix = Val; }

	// Transform setters and Getters
	DirectX::XMFLOAT3 GetPosition() const { return Position; }
	void SetPosition(DirectX::XMFLOAT3 Val)
	{
		Position = Val;
		UpdateWorldMatrix();
	}

	DirectX::XMFLOAT3 GetRotation() const { return Rotation; }
	void SetRotation(DirectX::XMFLOAT3 Val)
	{
		Rotation = Val;
		UpdateWorldMatrix();
	}

	DirectX::XMFLOAT3 GetScale() const { return Scale; }
	void SetScale(DirectX::XMFLOAT3 Val)
	{
		Scale = Val;
		UpdateWorldMatrix();
	}

	void UpdateWorldMatrix();

	// Direction vectors from transform
	DirectX::XMFLOAT3 GetForwardVector() const;
	DirectX::XMFLOAT3 GetRightVector() const;
	DirectX::XMFLOAT3 GetUpVector() const;

protected:
	DirectX::XMMATRIX WorldMatrix;

	// Mesh Transform
	DirectX::XMFLOAT3 Position = { 0,0,0 };
	DirectX::XMFLOAT3 Rotation = { 0,0,0 };
	DirectX::XMFLOAT3 Scale = { 1,1,1 };
};
