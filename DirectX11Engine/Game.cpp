//
// Game.cpp
//

#include "pch.h"
#include "Mesh.h"
#include "Cube.h"
#include "Camera.h"
#include "GameInputManager.h"
#include "Game.h"


extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
    Window(nullptr),
    OutputWidth(800),
    OutputHeight(600),
    FeatureLevel(D3D_FEATURE_LEVEL_9_1)
{
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    Window = window;
    OutputWidth = std::max(width, 1);
    OutputHeight = std::max(height, 1);

    CreateDevice();

    CreateResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
    if (!InputManager)
    {
        InputManager = new GameInputManager();
    }
    InputManager->Initialize(Window, this);
}

// Executes the basic game loop.
void Game::Tick()
{
    Timer.Tick([&]()
    {
        Update(Timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.

    FrameTime = elapsedTime;

    InputManager->Update();
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (Timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    // Draw each mesh of the scene
    for (auto Mesh : Meshes)
	{
		WorldViewProj = Mesh->GetWorldMatrix() * SceneCamera->GetViewMatrix() * SceneCamera->GetProjectionMatrix();

		PerObjectConstantBuffer.WorldViewProj = XMMatrixTranspose(WorldViewProj);
        PerObjectConstantBuffer.World = XMMatrixTranspose(Mesh->GetWorldMatrix());

		PerFrameConstantBuffer.SceneLight = *SceneLight;
		
		D3dContext->UpdateSubresource(PerFrameBuffer.Get(), 0, NULL, &PerFrameConstantBuffer, 0, 0);
		D3dContext->PSSetConstantBuffers(0, 1, PerFrameBuffer.GetAddressOf());

		D3dContext->UpdateSubresource(PerObjectBuffer.Get(), 0, NULL, &PerObjectConstantBuffer, 0, 0);
		D3dContext->VSSetConstantBuffers(0, 1, PerObjectBuffer.GetAddressOf());
		
		Mesh->Draw(D3dContext);
    }

	// Text
	Batch->Begin(SpriteSortMode_Texture);
	wchar_t TextToRender[50] = L"";
    swprintf(TextToRender, L"FPS : %.0f", 1/FrameTime);
	Font->DrawString(Batch.get(), TextToRender, FontPos, Colors::White, 0.f, Vector2(OutputWidth, OutputHeight), .5);
    Batch->End();

    Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    D3dContext->ClearRenderTargetView(RenderTargetView.Get(), Colors::Black);
    D3dContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    D3dContext->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), DepthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(OutputWidth), static_cast<float>(OutputHeight));
    D3dContext->RSSetViewports(1, &viewport);

    D3dContext->RSSetState(SolidState);
    D3dContext->OMSetDepthStencilState(DepthStencilState.Get(), 0);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = SwapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    Timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int width, int height)
{
    OutputWidth = std::max(width, 1);
    OutputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels [] =
    {
        // TODO: Modify for supported Direct3D feature levels
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    DX::ThrowIfFailed(D3D11CreateDevice(
        nullptr,                            // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
        &FeatureLevel,                    // returns feature level of device created
        context.ReleaseAndGetAddressOf()    // returns the device immediate context
        ));

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			/*d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_MESSAGE, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);*/
#endif
            D3D11_MESSAGE_ID hide [] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    DX::ThrowIfFailed(device.As(&D3dDevice));
    DX::ThrowIfFailed(context.As(&D3dContext));

    // TODO: Initialize device dependent objects here (independent of window size).
    Font = std::make_unique<SpriteFont>(D3dDevice.Get(), L"Assets/Fonts/Font.spriteFont");
    Batch = std::make_unique<SpriteBatch>(D3dContext.Get());

    // Initialize the camera
    SceneCamera = new Camera();
    SceneCamera->SetPosition(XMVectorSet(0.0f, 3.0f, -7.0f, 0.0f));
    
    // Create the meshes
    Cube* NewCube = new Cube();
    NewCube->SetPosition(XMFLOAT3(0, 0, 0));
    NewCube->InitMesh(D3dDevice, D3dContext);

    Cube* NewCube2 = new Cube();
    NewCube2->SetPosition(XMFLOAT3(0, 0, 3));
    NewCube2->InitMesh(D3dDevice, D3dContext);

    Meshes.push_back(NewCube);
    Meshes.push_back(NewCube2);

    // Create the lights
    SceneLight = new Light();
    SceneLight->Position = XMFLOAT3(0, 5, 0);
    SceneLight->Range = 100;
    SceneLight->Attenuation = XMFLOAT3(0.0, 0.2, 0.0);
    SceneLight->Direction = XMFLOAT3(0.25f, 0.5f, -1.0f);
    SceneLight->AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    SceneLight->DiffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    D3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    RenderTargetView.Reset();
    DepthStencilView.Reset();
    D3dContext->Flush();

    const UINT backBufferWidth = static_cast<UINT>(OutputWidth);
    const UINT backBufferHeight = static_cast<UINT>(OutputHeight);
    const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    constexpr UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (SwapChain)
    {
        HRESULT hr = SwapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(D3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBufferCount;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a SwapChain from a Win32 window.
        DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            D3dDevice.Get(),
            Window,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            SwapChain.ReleaseAndGetAddressOf()
            ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(Window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(SwapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(D3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, RenderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(D3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(D3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, DepthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.
    SceneCamera->UpdateProjectionMatrix(OutputWidth, OutputHeight);

    // Constant buffer
    D3D11_BUFFER_DESC ConstantBufferDescriptor;
    ZeroMemory(&ConstantBufferDescriptor, sizeof(D3D11_BUFFER_DESC));
    ConstantBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
    ConstantBufferDescriptor.ByteWidth = sizeof(PerObjectConstantBuffer);
    ConstantBufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ConstantBufferDescriptor.CPUAccessFlags = 0;
    ConstantBufferDescriptor.MiscFlags = 0;
    DX::ThrowIfFailed(D3dDevice->CreateBuffer(&ConstantBufferDescriptor, NULL, PerObjectBuffer.GetAddressOf()));

	ZeroMemory(&ConstantBufferDescriptor, sizeof(D3D11_BUFFER_DESC));
	ConstantBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
	ConstantBufferDescriptor.ByteWidth = sizeof(PerFrameConstantBuffer);
	ConstantBufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ConstantBufferDescriptor.CPUAccessFlags = 0;
	ConstantBufferDescriptor.MiscFlags = 0;
    DX::ThrowIfFailed(D3dDevice->CreateBuffer(&ConstantBufferDescriptor, NULL, PerFrameBuffer.GetAddressOf()));

    FontPos.x = backBufferWidth / 2.0f;
    FontPos.y = backBufferHeight / 2.0f;

    // Rasterizer States
    D3D11_RASTERIZER_DESC RasterizerDesc;

    ZeroMemory(&RasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
    RasterizerDesc.FillMode = D3D11_FILL_SOLID;
    RasterizerDesc.CullMode = D3D11_CULL_NONE;
    DX::ThrowIfFailed(D3dDevice->CreateRasterizerState(&RasterizerDesc, &SolidState));

	ZeroMemory(&RasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	RasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	RasterizerDesc.CullMode = D3D11_CULL_NONE;
	DX::ThrowIfFailed(D3dDevice->CreateRasterizerState(&RasterizerDesc, &WireFrameState));

    D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
    ZeroMemory(&DepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	// Depth test parameters
	DepthStencilDesc.DepthEnable = true;
	DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	DepthStencilDesc.StencilEnable = true;
	DepthStencilDesc.StencilReadMask = 0xFF;
	DepthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	DepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	DepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    DX::ThrowIfFailed(D3dDevice->CreateDepthStencilState(&DepthStencilDesc, DepthStencilState.GetAddressOf()));
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    for (auto Mesh : Meshes)
    {
        delete Mesh;
    }
    delete SceneLight;
    DepthStencilView.Reset();
    RenderTargetView.Reset();
    PerObjectBuffer->Release();
    PerFrameBuffer->Release();
    SolidState->Release();
    DepthStencilState->Release();
    WireFrameState->Release();
    SwapChain.Reset();
    D3dContext.Reset();
    D3dDevice.Reset();
    Font.reset();
    Batch.reset();

    CreateDevice();

    CreateResources();
}
