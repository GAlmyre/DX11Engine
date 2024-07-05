#pragma once
// The base class for a mesh
#include <vector>
#include "Material.h"
#include "Core/Actor.h"
#include "Core/pch.h"

using namespace DirectX::SimpleMath;


struct VertexType
{
	VertexType() = default;

	VertexType(const VertexType&) = default;
	VertexType& operator=(const VertexType&) = default;

	VertexType(VertexType&&) = default;
	VertexType& operator=(VertexType&&) = default;

	VertexType(DirectX::XMFLOAT3 const& iPosition, DirectX::XMFLOAT3 const& iNormal, DirectX::XMFLOAT3 const& iTangent, DirectX::XMFLOAT3 const& iBinormal, DirectX::XMFLOAT2 const& iTextureCoordinate) noexcept
		: Position(iPosition),
		Normal(iNormal),
		Tangent(iTangent),
		Binormal(iBinormal),
		TextureCoordinate(iTextureCoordinate)
	{ }

	VertexType(DirectX::FXMVECTOR iPosition, DirectX::FXMVECTOR iNormal, DirectX::FXMVECTOR iTangent, DirectX::FXMVECTOR iBinormal, DirectX::FXMVECTOR iTextureCoordinate) noexcept
	{
		XMStoreFloat3(&this->Position, iPosition);
		XMStoreFloat3(&this->Normal, iNormal);
		XMStoreFloat3(&this->Tangent, iNormal);
		XMStoreFloat3(&this->Binormal, iNormal);
		XMStoreFloat2(&this->TextureCoordinate, iTextureCoordinate);
	}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT3 Binormal;
	DirectX::XMFLOAT2 TextureCoordinate;

	static const int InputElementCount = 5;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

class Shader;

class Mesh : public Actor
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
	ID3D11ShaderResourceView* AlbedoTexture;
	ID3D11ShaderResourceView* NormalMap;
	MaterialData Material;
	ID3D11SamplerState* TextureSamplerState;
	std::wstring TexturePath;
	std::wstring NormalMapPath;

	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

	void AddVertex(DirectX::XMFLOAT3 Vertex, DirectX::XMFLOAT2 TextureCoord, DirectX::XMFLOAT3 Normal, DirectX::XMFLOAT3 Tangent, DirectX::XMFLOAT3 Binormal);

	void AddIndex(DWORD NewIndex);

	void SetMaterial(MaterialData MatData);

	// Initialise shaders and buffers for this mesh
	void InitMesh(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);

	void InitTextures(Microsoft::WRL::ComPtr<ID3D11Device1>& Device);

	void InitVertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);

	// Render the mesh
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);
};

