//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include "Light.h"

struct ConstantBufferPerFrame
{
    Light SceneLight;
};

struct ConstantBufferPerObject
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

	// ***  TODO : SCENE CLASS ***
	std::vector<class Mesh*> Meshes;
	Light* SceneLight;
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

    // Constant buffer to hold our multiplied matrices
    Microsoft::WRL::ComPtr<ID3D11Buffer> PerObjectBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> PerFrameBuffer;

    ConstantBufferPerObject PerObjectConstantBuffer;
    ConstantBufferPerFrame PerFrameConstantBuffer;

    DirectX::XMMATRIX WorldViewProj;

    // RenderText
    DirectX::SimpleMath::Vector2 FontPos;
    std::unique_ptr<DirectX::SpriteFont> Font;
    std::unique_ptr<DirectX::SpriteBatch> Batch;

    float FrameTime;
};
