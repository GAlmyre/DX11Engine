#pragma once
// The base class for a mesh
#include <vector>
#include "Material.h"

using namespace DirectX::SimpleMath;

using VertexType = DirectX::VertexPositionNormalTexture;

class Shader;

class Mesh
{
public:

	Mesh();

	Mesh(std::vector<VertexType> Vertices, std::vector<DWORD> Indices);
	// Create and initialize a mesh from assimp gathered data
	Mesh(aiMesh* AssimpMesh, const aiNode* Node, const aiScene* Scene, const std::wstring& ContainingFolder);

	~Mesh();

	// Vertices coordinates
	std::vector<VertexType> Vertices;

	// Indices
	std::vector<DWORD> Indices;

	// Texture and material
	ID3D11ShaderResourceView* Texture;
	MaterialData Material;
	ID3D11SamplerState* TextureSamplerState;
	std::wstring TexturePath = L""; // VERY TEMP

	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

	void AddVertex(DirectX::XMFLOAT3 Vertex, DirectX::XMFLOAT2 TextureCoord, DirectX::XMFLOAT3 Normal);

	void AddIndex(DWORD NewIndex);

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

	void SetMaterial(MaterialData MatData);

	void UpdateWorldMatrix();

	// Initialise shaders and buffers for this mesh
	void InitMesh(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);

	void InitTextures(Microsoft::WRL::ComPtr<ID3D11Device1>& Device);

	void InitVertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);

	// Render the mesh
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);

protected:
	DirectX::XMMATRIX WorldMatrix;

	// Mesh Transform
	DirectX::XMFLOAT3 Position = { 0,0,0 };
	DirectX::XMFLOAT3 Rotation = { 0,0,0 };
	DirectX::XMFLOAT3 Scale = { 1,1,1 };
};

