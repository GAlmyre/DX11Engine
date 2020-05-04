#pragma once
#include "Mesh.h"

// A simple cube mesh for testing purposes
class Cube :
	public Mesh
{
public:
	Cube();

	Cube(XMFLOAT3 Position, XMFLOAT3 Rotation, XMFLOAT3 Scale);

	void SetVerticesAndIndices();

};

