#include "Shader.h"
#include <d3dcompiler.h>

Shader::Shader(LPCWSTR Path, EShaderType InShaderType, Microsoft::WRL::ComPtr<ID3D11Device> Device)
{
	LPCSTR ShaderTypeString;
	ShaderType = InShaderType;

	switch (InShaderType)
	{
	case PixelShader:
		ShaderTypeString = "ps_5_0";
		break;
	case VertexShader:
		ShaderTypeString = "vs_5_0";
		break;
	default:
		break;
	}

	
	ID3DBlob* ShaderErrorMessage = nullptr;
	HRESULT hr = D3DCompileFromFile(Path, nullptr, nullptr, "main", ShaderTypeString, 0, 0, ShaderBuffer.ReleaseAndGetAddressOf(), nullptr);
	if (FAILED(hr))
	{
		const char* errorMsg = (const char*)ShaderErrorMessage->GetBufferPointer();
		MessageBox(nullptr, (LPCWSTR)errorMsg, L"Shader Compilation Error", MB_RETRYCANCEL);
		ShaderErrorMessage->Release();
	}


	switch (InShaderType)
	{
	case PixelShader:
	{
		Microsoft::WRL::ComPtr<ID3D11PixelShader> spPixelShader;
		DX::ThrowIfFailed(Device->CreatePixelShader(ShaderBuffer->GetBufferPointer(), ShaderBuffer->GetBufferSize(), nullptr, spPixelShader.GetAddressOf()));
		ShaderRef = spPixelShader;
	}
		break;
	case VertexShader:
	{
		Microsoft::WRL::ComPtr<ID3D11VertexShader> spVertexShader;
		DX::ThrowIfFailed(Device->CreateVertexShader(ShaderBuffer->GetBufferPointer(), ShaderBuffer->GetBufferSize(), nullptr, spVertexShader.GetAddressOf()));
		ShaderRef = spVertexShader;
	}
	break;
	default:
		break;
	}
	
}

Shader::~Shader()
{
	ShaderBuffer->Release();
	ShaderRef->Release();
}

Microsoft::WRL::ComPtr<ID3D11VertexShader> Shader::GetVertexShaderRef()
{
	Microsoft::WRL::ComPtr<ID3D11VertexShader> OutRef;
	HRESULT hr = ShaderRef.Get()->QueryInterface(IID_PPV_ARGS(&OutRef));
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"Failed VertexShader Cast", L"Shader Compilation Error", MB_RETRYCANCEL);
	}

	return OutRef;
}

Microsoft::WRL::ComPtr<ID3D11PixelShader> Shader::GetPixelShaderRef()
{
	Microsoft::WRL::ComPtr<ID3D11PixelShader> OutRef;
	HRESULT hr = ShaderRef.Get()->QueryInterface(IID_PPV_ARGS(&OutRef));
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"Failed PixelShader Cast", L"Shader Compilation Error", MB_RETRYCANCEL);
	}

	return OutRef;
}
