#include "Core/pch.h"
#include <d3dcompiler.h>
#include "Mesh.h"
#include <iostream>
#include "Core/Shader.h"
#include <Core/Math.h>
#include <filesystem>

using namespace DirectX;

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

Mesh::Mesh(aiMesh* AssimpMesh, const aiNode* Node, const aiScene* Scene, const std::wstring& ContainingFolder)
{
	aiVector3D NewPosition, NewRotation, NewScale;
	Node->mTransformation.Decompose(NewScale, NewRotation, NewPosition);
	SetPosition(XMFLOAT3(NewPosition.x, NewPosition.y, NewPosition.z));
	SetRotation(XMFLOAT3(Math::RadianToDegrees(NewRotation.x), Math::RadianToDegrees(NewRotation.y), Math::RadianToDegrees(NewRotation.z)));
	SetScale(XMFLOAT3(NewScale.x, NewScale.y, NewScale.z));

	UpdateWorldMatrix();

	// Add VertPos, TexCoord and Normal for each Vertex 
	for (UINT32 iVert = 0; iVert < AssimpMesh->mNumVertices; ++iVert)
	{
		XMFLOAT3 VertPos = { AssimpMesh->mVertices[iVert].x, AssimpMesh->mVertices[iVert].y, AssimpMesh->mVertices[iVert].z };
		XMFLOAT2 TexCoord = XMFLOAT2(0, 0);
		XMFLOAT3 Normal = XMFLOAT3(0, 0, 0);
		XMFLOAT3 Tangent = XMFLOAT3(0, 0, 0); 
		XMFLOAT3 Binormal = XMFLOAT3(0, 0, 0);

		if (AssimpMesh->mTextureCoords[0])
		{
			TexCoord = { AssimpMesh->mTextureCoords[0][iVert].x, AssimpMesh->mTextureCoords[0][iVert].y };
		}
		if (AssimpMesh->HasNormals())
		{
			Normal = { AssimpMesh->mNormals[iVert].x, AssimpMesh->mNormals[iVert].y, AssimpMesh->mNormals[iVert].z };	
		}
		if (AssimpMesh->HasTangentsAndBitangents())
		{
			Tangent = { AssimpMesh->mTangents[iVert].x, AssimpMesh->mTangents[iVert].y, AssimpMesh->mTangents[iVert].z };
			Binormal = { AssimpMesh->mBitangents[iVert].x, AssimpMesh->mBitangents[iVert].y, AssimpMesh->mBitangents[iVert].z };
		}

		AddVertex(VertPos, TexCoord, Normal, Tangent, Binormal);
	}

	// Add indices
	for (UINT32 iFaces = 0; iFaces < AssimpMesh->mNumFaces; ++iFaces)
	{
		for (UINT32 iIndex = 0; iIndex < AssimpMesh->mFaces->mNumIndices; ++iIndex)
		{
			AddIndex(AssimpMesh->mFaces[iFaces].mIndices[iIndex]);
		}
	}

	// Get Materials
	if (Scene->HasMaterials())
	{
		MaterialData Mat;
		aiColor3D DiffuseColor;
		aiColor3D AmbientColor;
		aiColor3D SpecularColor;
		float Shininess;

		aiReturn Res;
		Res = Scene->mMaterials[AssimpMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor);
		Res = Scene->mMaterials[AssimpMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor);
		Res = Scene->mMaterials[AssimpMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor);
		Res = Scene->mMaterials[AssimpMesh->mMaterialIndex]->Get(AI_MATKEY_SHININESS, Shininess);

		Mat.DiffuseColor = XMFLOAT4(DiffuseColor.r, DiffuseColor.g, DiffuseColor.b, 1.0f);
		Mat.AmbientColor = XMFLOAT4(AmbientColor.r, AmbientColor.g, AmbientColor.b, 1.0f);
		Mat.SpecularColor = XMFLOAT4(SpecularColor.r, SpecularColor.g, SpecularColor.b, 1.0f);

		Mat.SpecExp = Shininess <= 0.0f ? 32 : Shininess;

		// Albedo texture
		if (Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString AssimpTexturePath;
			Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &AssimpTexturePath);
			std::string StrTexturePath = std::string(AssimpTexturePath.C_Str());

			std::filesystem::path Path(StrTexturePath);
			std::wstring WidePath;
			if (Path.is_relative())
			{
				WidePath = ContainingFolder;
			}
			WidePath.append(DX::StringToWString(StrTexturePath));

			TexturePath = WidePath;
			Mat.bUseAlbedoTexture = true;
		}
		else
		{
			Mat.bUseAlbedoTexture = false;
		}

		// Normal Map (it can be called height or normals depending on the file format)
		if (Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			aiString AssimpTexturePath;
			Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTexture(aiTextureType_NORMALS, 0, &AssimpTexturePath);
			std::string StrTexturePath = std::string(AssimpTexturePath.C_Str());

			std::filesystem::path Path(StrTexturePath);
			std::wstring WidePath;
			if (Path.is_relative())
			{
				WidePath = ContainingFolder;
			}
			WidePath.append(DX::StringToWString(StrTexturePath));

			NormalMapPath = WidePath;
			Mat.bUseNormalMap = true;
		}
		else if (Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTextureCount(aiTextureType_HEIGHT) > 0)
		{
			aiString AssimpTexturePath;
			Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTexture(aiTextureType_HEIGHT, 0, &AssimpTexturePath);
			std::string StrTexturePath = std::string(AssimpTexturePath.C_Str());

			std::filesystem::path Path(StrTexturePath);
			std::wstring WidePath;
			if (Path.is_relative())
			{
				WidePath = ContainingFolder;
			}
			WidePath.append(DX::StringToWString(StrTexturePath));

			NormalMapPath = WidePath;
			Mat.bUseNormalMap = true;
		}
		else
		{
			Mat.bUseNormalMap = false;
		}

		// Specular Map
		if (Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			aiString AssimpTexturePath;
			Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTexture(aiTextureType_SPECULAR, 0, &AssimpTexturePath);
			std::string StrTexturePath = std::string(AssimpTexturePath.C_Str());

			std::filesystem::path Path(StrTexturePath);
			std::wstring WidePath;
			if (Path.is_relative())
			{
				WidePath = ContainingFolder;
			}
			WidePath.append(DX::StringToWString(StrTexturePath));

			SpecularMapPath = WidePath;
			Mat.bUseSpecularMap = true;
		}
		else
		{
			Mat.bUseSpecularMap = false;
		}

		SetMaterial(Mat);
	}
}

Mesh::~Mesh()
{
	VertexBuffer->Release();
	IndexBuffer->Release();
	Vertices.clear();
}

void Mesh::AddVertex(DirectX::XMFLOAT3 Vertex, DirectX::XMFLOAT2 TextureCoord, DirectX::XMFLOAT3 Normal, DirectX::XMFLOAT3 Tangent, DirectX::XMFLOAT3 Binormal)
{
	VertexType NewVertex;
	NewVertex.Position = Vertex;
	NewVertex.TextureCoordinate = TextureCoord;
	NewVertex.Normal = Normal;
	NewVertex.Tangent = Tangent;
	NewVertex.Binormal = Binormal;

	Vertices.push_back(NewVertex);
}

void Mesh::AddIndex(DWORD NewIndex)
{
	Indices.push_back(NewIndex);
}

void Mesh::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext)
{
	// Set Vertex/Index Buffer
	UINT stride = sizeof(VertexType);
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);
	DeviceContext->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set Texture
	if (!TexturePath.empty())
	{
		DeviceContext.Get()->PSSetShaderResources(0, 1, &AlbedoTexture);
		DeviceContext.Get()->PSSetSamplers(0, 1, &TextureSamplerState);
	}
	if (!NormalMapPath.empty())
	{
		DeviceContext.Get()->PSSetShaderResources(1, 1, &NormalMap);
	}
	if (!SpecularMapPath.empty())
	{
		DeviceContext.Get()->PSSetShaderResources(2, 1, &SpecularMap);
	}

	// Draw
	DeviceContext->DrawIndexed((UINT)Indices.size(), 0, 0);
}

void Mesh::SetMaterial(MaterialData MatData)
{
	Material = MatData;
}

void Mesh::InitMesh(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext)
{
	InitTextures(Device, DeviceContext);

	InitVertexBuffer(Device, DeviceContext);
}

void Mesh::InitTextures(Microsoft::WRL::ComPtr<ID3D11Device1>& Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext)
{
	if (!TexturePath.empty())
	{
		// Init textures
		HRESULT Hr = CreateWICTextureFromFile(Device.Get(), DeviceContext.Get(), TexturePath.c_str(), nullptr, &AlbedoTexture);	
		if (Hr != E_FAIL)
		{
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
		else
		{
			Logger::Log("Failed to create texture " + DX::WStringToString(TexturePath), LogSeverity::Warning);
			TexturePath = L"";
		}
	}

	if (!NormalMapPath.empty())
	{
		// Init textures
		HRESULT Hr = CreateWICTextureFromFile(Device.Get(), DeviceContext.Get(), NormalMapPath.c_str(), nullptr, &NormalMap);	
		if (Hr != E_FAIL)
		{
			
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
		else
		{

			Logger::Log("Failed to create texture " + DX::WStringToString(NormalMapPath), LogSeverity::Warning);
			NormalMapPath = L"";
		}
	}

	if (!SpecularMapPath.empty())
	{
		// Init textures
		HRESULT Hr = CreateWICTextureFromFile(Device.Get(), DeviceContext.Get(), SpecularMapPath.c_str(), nullptr, &SpecularMap);
		if (Hr != E_FAIL)
		{

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
		else
		{

			Logger::Log("Failed to create texture " + DX::WStringToString(SpecularMapPath), LogSeverity::Warning);
			SpecularMapPath = L"";
		}
	}
}

void Mesh::InitVertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device1> Device, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> DeviceContext)
{
	// Create the VertexBuffer
	D3D11_BUFFER_DESC VertexBufferDesc;
	ZeroMemory(&VertexBufferDesc, sizeof(VertexBufferDesc));

	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.ByteWidth = sizeof(VertexType) * (UINT)Vertices.size();
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
	IndexBufferDesc.ByteWidth = sizeof(DWORD) * (UINT)Indices.size();
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA IndexBufferData;
	ZeroMemory(&IndexBufferData, sizeof(IndexBufferData));
	IndexBufferData.pSysMem = &Indices[0];

	DX::ThrowIfFailed(Device->CreateBuffer(&IndexBufferDesc, &IndexBufferData, IndexBuffer.GetAddressOf()));
}