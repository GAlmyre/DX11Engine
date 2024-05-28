#pragma once
#include "Core/pch.h"

class Actor
{
public:
	// World Matrix
	DirectX::XMMATRIX GetWorldMatrix() const { return WorldMatrix; }
	void SetWorldMatrix(DirectX::XMMATRIX val) { WorldMatrix = val; }

	// Transform setters and Getters
	DirectX::XMFLOAT3 GetPosition() const { return Position; }
	void SetPosition(DirectX::XMFLOAT3 val)
	{
		Position = val;
		UpdateWorldMatrix();
	}

	DirectX::XMFLOAT3 GetRotation() const { return Rotation; }
	void SetRotation(DirectX::XMFLOAT3 val)
	{
		Rotation = val;
		UpdateWorldMatrix();
	}

	DirectX::XMFLOAT3 GetScale() const { return Scale; }
	void SetScale(DirectX::XMFLOAT3 val)
	{
		Scale = val;
		UpdateWorldMatrix();
	}

	void UpdateWorldMatrix();

protected:
	DirectX::XMMATRIX WorldMatrix;

	// Mesh Transform
	DirectX::XMFLOAT3 Position = { 0,0,0 };
	DirectX::XMFLOAT3 Rotation = { 0,0,0 };
	DirectX::XMFLOAT3 Scale = { 1,1,1 };
};
