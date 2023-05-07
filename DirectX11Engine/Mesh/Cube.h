#pragma once
#include "Mesh.h"

// A simple cube mesh for testing purposes
class Cube :
	public Mesh
{
public:
	Cube();

	Cube(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Rotation, DirectX::XMFLOAT3 Scale);

	void SetVerticesAndIndices();

};

