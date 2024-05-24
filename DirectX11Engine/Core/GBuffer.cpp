#include "GBuffer.h"

GBuffer::GBuffer()
{
	Reset();
}

GBuffer::~GBuffer()
{
	Reset();
}

bool GBuffer::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> Device, int InTextureWidth, int InTextureHeight, float ScreenFar, float ScreenNear)
{
	D3D11_TEXTURE2D_DESC TextureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	D3D11_TEXTURE2D_DESC DepthBuferDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;

	TextureWidth = InTextureWidth;
	TextureHeight = InTextureHeight;

	// Render target texture
	ZeroMemory(&TextureDesc, sizeof(TextureDesc));

	TextureDesc.Width = TextureWidth;
	TextureDesc.Height = TextureHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		DX::ThrowIfFailed(Device->CreateTexture2D(&TextureDesc, NULL, RenderTargetTextures[i].GetAddressOf()));
	}

	// Render Target View
	RTVDesc.Format = TextureDesc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;

	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		DX::ThrowIfFailed(Device->CreateRenderTargetView(RenderTargetTextures[i].Get(), &RTVDesc, RenderTargetViews[i].GetAddressOf()));
	}

	// Shader Resource View
	SRVDesc.Format = TextureDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 0;

	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		DX::ThrowIfFailed(Device->CreateShaderResourceView(RenderTargetTextures[i].Get(), &SRVDesc, ShaderResourceViews[i].GetAddressOf()));
	}

	// Depth Buffer
	ZeroMemory(&DepthBuferDesc, sizeof(DepthBuferDesc));

	DepthBuferDesc.Width = TextureWidth;
	DepthBuferDesc.Height = TextureHeight;
	DepthBuferDesc.MipLevels = 1;
	DepthBuferDesc.ArraySize = 1;
	DepthBuferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthBuferDesc.SampleDesc.Count = 1;
	DepthBuferDesc.SampleDesc.Quality = 0;
	DepthBuferDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthBuferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthBuferDesc.CPUAccessFlags = 0;
	DepthBuferDesc.MiscFlags = 0;

	DX::ThrowIfFailed(Device->CreateTexture2D(&DepthBuferDesc, NULL, DepthStencilBuffer.GetAddressOf()));

	ZeroMemory(&DepthStencilViewDesc, sizeof(DepthStencilViewDesc));

	DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DepthStencilViewDesc.Texture2D.MipSlice = 0;

	DX::ThrowIfFailed(Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &DepthStencilViewDesc, DepthStencilView.GetAddressOf()));

	Viewport->Width = (float)TextureWidth;
	Viewport->Height = (float)TextureHeight;
	Viewport->MinDepth = 0.0f;
	Viewport->MaxDepth = 1.0f;
	Viewport->TopLeftX = 0.0f;
	Viewport->TopLeftY = 0.0f;
	
	return true;
}

void GBuffer::SetRenderTargets(Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext)
{

}

void GBuffer::ClearRenderTargets(Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext, DirectX::XMVECTORF32 Color)
{

}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GBuffer::GetSRV(int Index)
{
	if (Index >= 0 && Index < BUFFER_COUNT)
	{
		return ShaderResourceViews[Index];
	}
}

void GBuffer::Reset()
{
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		RenderTargetTextures[i].Reset();
		RenderTargetViews[i].Reset();
		ShaderResourceViews[i].Reset();
	}

	DepthStencilBuffer.Reset();
	DepthStencilView.Reset();
}