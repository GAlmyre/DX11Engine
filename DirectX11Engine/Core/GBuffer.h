#include "pch.h"

const int BUFFER_COUNT = 2;

// A class to hold all the needed data to setup a GBuffer : RTVs, SRVs, Textures etc...
class GBuffer
{
public:
	GBuffer();
	~GBuffer();

	bool Initialize(Microsoft::WRL::ComPtr<ID3D11Device> Device, int TextureWidth, int TextureHeight, float ScreenFar, float ScreenNear);

	void SetRenderTargets(Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext);
	void ClearRenderTargets(Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext, DirectX::XMVECTORF32 Color);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV(int Index);

private:
	
	// Reset every ComPtr in the object
	void Reset();

	int TextureWidth;
	int TextureHeight;

	Microsoft::WRL::ComPtr<ID3D11Texture2D>				RenderTargetTextures[BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		RenderTargetViews[BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	ShaderResourceViews[BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				DepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		DepthStencilView;
	Microsoft::WRL::ComPtr<D3D11_VIEWPORT>				Viewport;
};