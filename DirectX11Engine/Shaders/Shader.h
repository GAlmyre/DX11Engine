#pragma once
#include "Core/pch.h"

enum EShaderType
{
	PixelShader,
	VertexShader
};

// A helper class to compiler or use shaders
class Shader
{
public:
	Shader(LPCWSTR Path, EShaderType ShaderType, Microsoft::WRL::ComPtr<ID3D11Device> Device);
	~Shader();

	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShaderRef();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShaderRef();

	Microsoft::WRL::ComPtr<ID3D11DeviceChild> ShaderRef;
	Microsoft::WRL::ComPtr<ID3D10Blob> ShaderBuffer;

	EShaderType ShaderType;
};