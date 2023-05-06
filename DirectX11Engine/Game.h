//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include "Light.h"
#include "Material.h"

struct ConstantBufferPerFrame_PS
{
    DirectionalLightData Sun = DirectionalLightData();
    PointLightData PointLights[MAX_LIGHTS];
    DirectX::XMFLOAT3 CameraPosition = DirectX::XMFLOAT3();
    float LightsCount = 0;
    //DirectX::XMFLOAT3 PadEnd = DirectX::XMFLOAT3(65, 65, 65);
   
};

struct ConstantBufferPerObject_PS
{
	MaterialData Mat = MaterialData();
    //int bUseTexture;
    //DirectX::XMFLOAT3 PadTex = DirectX::XMFLOAT3(0, 0, 0);
};

struct ConstantBufferPerObject_VS
{
    DirectX::XMMATRIX WorldViewProj;
    DirectX::XMMATRIX World;
};

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:

    Game() noexcept;
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

    void LoadNewModel(std::wstring Path);

	// ***  TODO : SCENE CLASS ***
	std::vector<class Mesh*> Meshes;

    // A scene can contain one directional light and MAX_LIGHTS PointLights
	DirectionalLight* Sun;
    std::vector<PointLight*> Lights;
	class Camera* SceneCamera = nullptr;
	// ***  SCENE CLASS ***

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

    // Device resources.
    HWND                                            Window;
    int                                             OutputWidth;
    int                                             OutputHeight;

    D3D_FEATURE_LEVEL                               FeatureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device1>           D3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    D3dContext;

    Microsoft::WRL::ComPtr<IDXGISwapChain1>         SwapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  RenderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  DepthStencilView;

    // RasterizerStates
    ID3D11RasterizerState* SolidState;
    ID3D11RasterizerState* WireFrameState;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState;

    // Rendering loop timer.
    DX::StepTimer                                   Timer;

    class GameInputManager* InputManager = nullptr;
   

    float Pitch = 0;
    float Yaw = 0;

    // ***** TODO : Where to put that ? *****

    // Constant buffers for vertex and pixel shader
    Microsoft::WRL::ComPtr<ID3D11Buffer> PerObjectBuffer_VS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> PerFrameBuffer_PS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> PerObjectBuffer_PS;

    ConstantBufferPerObject_VS PerObjectBuffStruct_VS;
    ConstantBufferPerFrame_PS PerFrameBuffStruct_PS;
    ConstantBufferPerObject_PS PerObjectBuffStruct_PS;

    // ***** TODO : Where to put that ? *****

    DirectX::XMMATRIX WorldViewProj;

    // RenderText
    DirectX::SimpleMath::Vector2 FontPos;
    std::unique_ptr<DirectX::SpriteFont> Font;
    std::unique_ptr<DirectX::SpriteBatch> Batch;

    float FrameTime;
};
