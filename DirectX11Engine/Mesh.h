#pragma once
// The base class for a mesh
#include <vector>

using namespace DirectX;
using namespace DirectX::SimpleMath;

using VertexType = DirectX::VertexPositionNormalTexture;

class Mesh
{
public:

	Mesh();

	Mesh(std::vector<VertexType> Vertices, std::vector<DWORD> Indices);

	~Mesh();

	// Vertices coordinates
	std::vector<VertexType> Vertices;

	// Indices
	std::vector<DWORD> Indices;

	// Texture
	ID3D11ShaderResourceView* Texture;
	ID3D11SamplerState* TextureSamplerState;
	LPCTSTR TexturePath;

	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

	// Input layout
	Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;

	// Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
	Microsoft::WRL::ComPtr<ID3D10Blob> VertexShader_Buffer;
	Microsoft::WRL::ComPtr<ID3D10Blob> PixelShader_Buffer;

	// World Matrix
	XMMATRIX GetWorldMatrix() const { return WorldMatrix; }
	void SetWorldMatrix(XMMATRIX val) { WorldMatrix = val; }

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

	// Initialise shaders and buffers for this mesh
	void InitMesh(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);

	void InitTextures(Microsoft::WRL::ComPtr<ID3D11Device1>& Device);

	void InitInputLayout(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);

	void InitVertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);

	void InitShaders(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);

	// Render the mesh
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext);

protected:
	XMMATRIX WorldMatrix;

	// Mesh Transform
	XMFLOAT3 Position = { 0,0,0 };
	XMFLOAT3 Rotation = { 0,0,0 };
	XMFLOAT3 Scale = { 1,1,1 };
};

