#include "pch.h"
#include "Cube.h"

using namespace DirectX;

Cube::Cube()
	:Mesh()
{
	SetVerticesAndIndices();
	TexturePath = L"Assets/Textures/container.png";
}

Cube::Cube(XMFLOAT3 Position, XMFLOAT3 Rotation, XMFLOAT3 Scale)
{
	SetVerticesAndIndices();

	this->Position = Position;
	this->Rotation = Rotation;
	this->Scale = Scale;

	UpdateWorldMatrix();
	TexturePath = L"Assets/Textures/container.png";
}

void Cube::SetVerticesAndIndices()
{
	Vertices =
	{
		// Front Face
		VertexType(XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f)),
		VertexType(XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)),
		VertexType(XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)),
		VertexType(XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)),
												  
		//Back Face								  
		VertexType(XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)),
		VertexType(XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)),
		VertexType(XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)),
		VertexType(XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)),
												  
		// Top Face								  
		VertexType(XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f)),
		VertexType(XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)),
		VertexType(XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)),
		VertexType(XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)),
												  
		// Bottom Face							  
		VertexType(XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)),
		VertexType(XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f)),
		VertexType(XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)),
		VertexType(XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)),
												 
		// Left Face							  
		VertexType(XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)),
		VertexType(XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)),
		VertexType(XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)),
		VertexType(XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)),

		// Right Face
		VertexType(XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f)),
		VertexType(XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)),
		VertexType(XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)),
		VertexType(XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)),
			
	};

	Indices =
	{
		// Front Face
		0,  1,  2,
		0,  2,  3,

		// Back Face
		4,  5,  6,
		4,  6,  7,

		// Top Face
		8,  9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};
}
