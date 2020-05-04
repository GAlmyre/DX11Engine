#include "pch.h"
#include <d3dcompiler.h>
#include "Mesh.h"
#include <iostream>

Mesh::Mesh()
{
	SetWorldMatrix(XMMatrixIdentity());
}

Mesh::Mesh(std::vector<VertexType> Vertices, std::vector<DWORD> Indices)
{
	this->Vertices = Vertices;
	this->Indices = Indices;

	SetWorldMatrix(XMMatrixIdentity());
}

Mesh::~Mesh()
{
	InputLayout->Release();
	VertexBuffer->Release();
	IndexBuffer->Release();
	Vertices.clear();
	VertexShader->Release();
	PixelShader->Release();
	VertexShader_Buffer->Release();
	PixelShader_Buffer->Release();
}

void Mesh::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext)
{
	// Set shaders
	DeviceContext->VSSetShader(VertexShader.Get(), 0, 0);
	DeviceContext->PSSetShader(PixelShader.Get(), 0, 0);

	// Set Vertex/Index Buffer
	UINT stride = sizeof(VertexType);
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);
	DeviceContext->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set Input layout
	DeviceContext->IASetInputLayout(InputLayout.Get());
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set Texture
	DeviceContext.Get()->PSSetShaderResources(0, 1, &Texture);
	DeviceContext.Get()->PSSetSamplers(0, 1, &TextureSamplerState);

	// Draw
	DeviceContext->DrawIndexed(Indices.size(), 0, 0);
}

void Mesh::UpdateWorldMatrix()
{
	XMMATRIX TranslationMatrix = XMMatrixTranslation(Position.x, Position.y, Position.z);
	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, Rotation.z);
	XMMATRIX ScaleMatrix = XMMatrixScaling(Scale.x, Scale.y, Scale.z);

	WorldMatrix = ScaleMatrix * RotationMatrix * TranslationMatrix;
}

void Mesh::InitMesh(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext)
{
	InitShaders(Device, DeviceContext);

	InitTextures(Device);

	InitVertexBuffer(Device, DeviceContext);

	InitInputLayout(Device, DeviceContext);
}

void Mesh::InitTextures(Microsoft::WRL::ComPtr<ID3D11Device1>& Device)
{
	// Init textures
	DX::ThrowIfFailed(CreateWICTextureFromFile(Device.Get(), TexturePath, nullptr, &Texture));
	D3D11_SAMPLER_DESC SamplerDesc;
	ZeroMemory(&SamplerDesc, sizeof(SamplerDesc));
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	DX::ThrowIfFailed(Device.Get()->CreateSamplerState(&SamplerDesc, &TextureSamplerState));
}

void Mesh::InitInputLayout(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext)
{
	// Create and set the InputLayout
	D3D11_INPUT_ELEMENT_DESC Layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT LayoutNum = ARRAYSIZE(Layout);
	DX::ThrowIfFailed(Device->CreateInputLayout(Layout, LayoutNum, VertexShader_Buffer->GetBufferPointer(), VertexShader_Buffer->GetBufferSize(), InputLayout.GetAddressOf()));
}

void Mesh::InitVertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext)
{
	// Create the VertexBuffer
	D3D11_BUFFER_DESC VertexBufferDesc;
	ZeroMemory(&VertexBufferDesc, sizeof(VertexBufferDesc));

	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.ByteWidth = sizeof(VertexType) * Vertices.size();
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA VertexBufferData;
	ZeroMemory(&VertexBufferData, sizeof(VertexBufferData));
	VertexBufferData.pSysMem = &Vertices[0]; // needs the address of the array, not the vector

	HRESULT hr = Device->CreateBuffer(&VertexBufferDesc, &VertexBufferData, VertexBuffer.GetAddressOf());
	DX::ThrowIfFailed(hr);	

	// Create the IndexBuffer
	D3D11_BUFFER_DESC IndexBufferDesc;
	ZeroMemory(&IndexBufferDesc, sizeof(IndexBufferDesc));

	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.ByteWidth = sizeof(DWORD) * Indices.size();
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA IndexBufferData;
	ZeroMemory(&IndexBufferData, sizeof(IndexBufferData));
	IndexBufferData.pSysMem = &Indices[0];

	DX::ThrowIfFailed(Device->CreateBuffer(&IndexBufferDesc, &IndexBufferData, IndexBuffer.GetAddressOf()));
}

void Mesh::InitShaders(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext)
{
	// Compile the shaders from file	
	ID3DBlob* ShaderErrorMessage = nullptr;
	HRESULT hr = D3DCompileFromFile(L"Shaders/SimpleVertexShader.hlsl", NULL, NULL, "main", "vs_4_0", 0, 0, VertexShader_Buffer.ReleaseAndGetAddressOf(), &ShaderErrorMessage);
	if (FAILED(hr) && ShaderErrorMessage)
	{
		const char* errorMsg = (const char*)ShaderErrorMessage->GetBufferPointer();
		MessageBox(nullptr, (LPCWSTR)errorMsg, L"Shader Compilation Error", MB_RETRYCANCEL);
		ShaderErrorMessage->Release();
	}
	hr = D3DCompileFromFile(L"Shaders/SimplePixelShader.hlsl", NULL, NULL, "main", "ps_4_0", 0, 0, PixelShader_Buffer.GetAddressOf(), 0);
	if (FAILED(hr) && ShaderErrorMessage)
	{
		const char* errorMsg = (const char*)ShaderErrorMessage->GetBufferPointer();
		MessageBox(nullptr, (LPCWSTR)errorMsg, L"Shader Compilation Error", MB_RETRYCANCEL);
		ShaderErrorMessage->Release();
	}

	// Create the shaders and set them
	DX::ThrowIfFailed(Device->CreateVertexShader(VertexShader_Buffer->GetBufferPointer(), VertexShader_Buffer->GetBufferSize(), NULL, VertexShader.GetAddressOf()));
	DX::ThrowIfFailed(Device->CreatePixelShader(PixelShader_Buffer->GetBufferPointer(), PixelShader_Buffer->GetBufferSize(), NULL, PixelShader.GetAddressOf()));
}
