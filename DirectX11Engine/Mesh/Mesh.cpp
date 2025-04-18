#include "Core/pch.h"
#include <d3dcompiler.h>
#include "Mesh.h"
#include <iostream>
#include "Shaders/Shader.h"
#include <Core/Math.h>

using namespace DirectX;

Mesh::Mesh()
{
	SetWorldMatrix(XMMatrixIdentity());
}

Mesh::Mesh(std::vector<VertexType> Vertices, std::vector<DWORD> Indices)
{
	this->Vertices = Vertices;
	this->Indices = Indices;
	TexturePath = L"Assets/Textures/DefaultTexture.png";
	NormalMapPath = L"Assets/Textures/DefaultBump.png";
	SetWorldMatrix(XMMatrixIdentity());
}

Mesh::Mesh(aiMesh* AssimpMesh, const aiNode* Node, const aiScene* Scene, const std::wstring& ContainingFolder)
{

	//XMMATRIX NewWorldMatrix = XMMATRIX(	Node->mTransformation.a1, Node->mTransformation.b1, Node->mTransformation.c1, Node->mTransformation.d1,
	//									Node->mTransformation.a2, Node->mTransformation.b2, Node->mTransformation.c2, Node->mTransformation.d2,
	//									Node->mTransformation.a3, Node->mTransformation.b3, Node->mTransformation.c3, Node->mTransformation.d3,
	//									Node->mTransformation.a4, Node->mTransformation.b4, Node->mTransformation.c4, Node->mTransformation.d4);
	//
	//

	//SetWorldMatrix(NewWorldMatrix);

	aiVector3D NewPosition, NewRotation, NewScale;
	Node->mTransformation.Decompose(NewScale, NewRotation, NewPosition);
	SetPosition(XMFLOAT3(NewPosition.x, NewPosition.y, NewPosition.z));
	SetRotation(XMFLOAT3(Math::RadianToDegrees(NewRotation.x), Math::RadianToDegrees(NewRotation.y), Math::RadianToDegrees(NewRotation.z)));
	SetScale(XMFLOAT3(NewScale.x, NewScale.y, NewScale.z));

	UpdateWorldMatrix();

	// Add VertPos, TexCoord and Normal for each Vertex 
	for (int iVert = 0; iVert < AssimpMesh->mNumVertices; ++iVert)
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
	for (int iFaces = 0; iFaces < AssimpMesh->mNumFaces; ++iFaces)
	{
		for (int iIndex = 0; iIndex < AssimpMesh->mFaces->mNumIndices; ++iIndex)
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

		Mat.DiffuseColor = XMFLOAT3(DiffuseColor.r, DiffuseColor.g, DiffuseColor.b);
		Mat.AmbientColor = XMFLOAT3(AmbientColor.r, AmbientColor.g, AmbientColor.b);
		Mat.SpecularColor = XMFLOAT3(SpecularColor.r, SpecularColor.g, SpecularColor.b);

		Mat.SpecExp = Shininess <= 0.0f ? 64 : Shininess;

		// Albedo texture
		if (Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString AssimpTexturePath;
			Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &AssimpTexturePath);
			std::string StrTexturePath = std::string(AssimpTexturePath.C_Str());
			std::wstring WidePath = ContainingFolder;
			WidePath.append(DX::StringToWString(StrTexturePath));

			TexturePath = WidePath;
		}
		else
		{
			TexturePath = L"Assets/Textures/DefaultTexture.png";
		}

		// Normal Map
		if (Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTextureCount(aiTextureType_HEIGHT) > 0)
		{
			aiString AssimpTexturePath;
			Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTexture(aiTextureType_HEIGHT, 0, &AssimpTexturePath);
			std::string StrTexturePath = std::string(AssimpTexturePath.C_Str());
			std::wstring WidePath = ContainingFolder;
			WidePath.append(DX::StringToWString(StrTexturePath));

			NormalMapPath = WidePath;
		}
		else
		{
			NormalMapPath = L"Assets/Textures/DefaultBump.png";
		}

		// Specular Map
		if (Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			aiString AssimpTexturePath;
			Scene->mMaterials[AssimpMesh->mMaterialIndex]->GetTexture(aiTextureType_SPECULAR, 0, &AssimpTexturePath);
			std::string StrTexturePath = std::string(AssimpTexturePath.C_Str());
			std::wstring WidePath = ContainingFolder;
			WidePath.append(DX::StringToWString(StrTexturePath));

			SpecularMapPath = WidePath;
		}
		else
		{
			SpecularMapPath = L"Assets/Textures/DefaultTexture.png";
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
	DeviceContext->DrawIndexed(Indices.size(), 0, 0);
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
	if (TexturePath != L"")
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
			TexturePath = L"";
		}
	}

	if (NormalMapPath != L"")
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
			NormalMapPath = L"";
		}
	}

	if (SpecularMapPath != L"")
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