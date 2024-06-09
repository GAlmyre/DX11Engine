//
// Renderer.cpp
//

#include "Core/pch.h"
#include "Mesh/Mesh.h"
#include "Mesh/Cube.h"
#include "Camera.h"
#include "GameInputManager.h"
#include "Renderer.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assimp/material.h"
#include "Shaders/Shader.h"

// GUI
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "Math.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Renderer::Renderer() noexcept :
    Window(nullptr),
    OutputWidth(800),
    OutputHeight(600),
    FeatureLevel(D3D_FEATURE_LEVEL_9_1)
{
}

// Initialize the Direct3D resources required to run.
void Renderer::Initialize(HWND window, int width, int height)
{
    Window = window;
    OutputWidth = std::max(width, 1);
    OutputHeight = std::max(height, 1);

    CreateDevice();

    CreateResources();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(D3dDevice.Get(), D3dContext.Get());

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
void Renderer::Tick()
{
    Timer.Tick([&]()
    {
        Update(Timer);
    });

    Render();
}

// Updates the world.
void Renderer::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    FrameTime = elapsedTime;

    InputManager->Update();
}

// Draws the scene.
void Renderer::Render()
{
    // Don't try to render anything before the first Update.
    if (Timer.GetFrameCount() == 0)
    {
        return;
    }

    DrawGui();


    Clear();

	// Clear the views
	D3dContext->ClearRenderTargetView(RenderTargetView.Get(), Colors::Aqua);

    // GBUffer Pass
	D3dContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	D3dContext->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), DepthStencilView.Get());

	// Set the viewport
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(OutputWidth), static_cast<float>(OutputHeight));
	D3dContext->RSSetViewports(1, &viewport);

	D3dContext->RSSetState(SolidState);
	D3dContext->OMSetDepthStencilState(DepthStencilState.Get(), 0);

	// Set Input layout
    D3dContext->IASetInputLayout(InputLayout.Get());
    D3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> VSRef = VertexShader->GetVertexShaderRef();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> PSRef = CurrentPixelShader->GetPixelShaderRef();

    D3dContext->VSSetShader(VSRef.Get(), 0, 0);
    D3dContext->PSSetShader(PSRef.Get(), 0, 0);

    // LightingPass

    // Draw each mesh of the scene
    for (Mesh* Mesh : Meshes)
	{
		WorldViewProj = Mesh->GetWorldMatrix() * SceneCamera->GetViewMatrix() * SceneCamera->GetProjectionMatrix();

		PerObjectBuffStruct_VS.WorldViewProj = XMMatrixTranspose(WorldViewProj);
        PerObjectBuffStruct_VS.World = XMMatrixTranspose(Mesh->GetWorldMatrix());

		PerFrameBuffStruct_PS.Sun = Sun->GetLightData();

		for (int i = 0; i < MAX_LIGHTS && i < Lights.size(); ++i)
		{
			   PerFrameBuffStruct_PS.PointLights[i] = Lights[i].Light->GetLightData();
		}
        PerObjectBuffStruct_PS.Mat = Mesh->Material;

		XMFLOAT3 CamPos{};
		XMStoreFloat3(&CamPos, SceneCamera->GetPosition());
		PerFrameBuffStruct_PS.CameraPosition = CamPos;
        PerFrameBuffStruct_PS.LightsCount = Lights.size();

		D3dContext->UpdateSubresource(PerFrameBuffer_PS.Get(), 0, nullptr, &PerFrameBuffStruct_PS, 0, 0);
		D3dContext->PSSetConstantBuffers(0, 1, PerFrameBuffer_PS.GetAddressOf());

		D3dContext->UpdateSubresource(PerObjectBuffer_PS.Get(), 0, nullptr, &PerObjectBuffStruct_PS, 0, 0);
		D3dContext->PSSetConstantBuffers(1, 1, PerObjectBuffer_PS.GetAddressOf());

		D3dContext->UpdateSubresource(PerObjectBuffer_VS.Get(), 0, nullptr, &PerObjectBuffStruct_VS, 0, 0);
		D3dContext->VSSetConstantBuffers(0, 1, PerObjectBuffer_VS.GetAddressOf());
		
		Mesh->Draw(D3dContext);
    }
    if (bDrawLightEmitters)
    {
		// Draw meshes for the lights
		for (LightAndMesh CurrentLight : Lights)
		{
			D3dContext->PSSetShader(UnlitPixelShader->GetPixelShaderRef().Get(), 0, 0);

            CurrentLight.LightMesh->InitMesh(D3dDevice, D3dContext);
			WorldViewProj = CurrentLight.LightMesh->GetWorldMatrix() * SceneCamera->GetViewMatrix() * SceneCamera->GetProjectionMatrix();

			PerObjectBuffStruct_VS.WorldViewProj = XMMatrixTranspose(WorldViewProj);
			PerObjectBuffStruct_VS.World = XMMatrixTranspose(CurrentLight.LightMesh->GetWorldMatrix());

			PerFrameBuffStruct_PS.Sun = Sun->GetLightData();

			XMFLOAT3 CamPos{};
			XMStoreFloat3(&CamPos, SceneCamera->GetPosition());
			PerFrameBuffStruct_PS.CameraPosition = CamPos;
			PerFrameBuffStruct_PS.LightsCount = Lights.size();

			D3dContext->UpdateSubresource(PerFrameBuffer_PS.Get(), 0, nullptr, &PerFrameBuffStruct_PS, 0, 0);
			D3dContext->PSSetConstantBuffers(0, 1, PerFrameBuffer_PS.GetAddressOf());

			D3dContext->UpdateSubresource(PerObjectBuffer_PS.Get(), 0, nullptr, &PerObjectBuffStruct_PS, 0, 0);
			D3dContext->PSSetConstantBuffers(1, 1, PerObjectBuffer_PS.GetAddressOf());

			D3dContext->UpdateSubresource(PerObjectBuffer_VS.Get(), 0, nullptr, &PerObjectBuffStruct_VS, 0, 0);
			D3dContext->VSSetConstantBuffers(0, 1, PerObjectBuffer_VS.GetAddressOf());

            CurrentLight.LightMesh->Draw(D3dContext);
		}
    }   

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    Present();
}

void Renderer::DrawGui()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

    ImGui::Begin("Settings");

    ImGui::SliderFloat("Camera Speed", &SceneCamera->Speed, 0.1f, 50.0f);
	static float SunDiffuseColor[3] = { Sun->DiffuseColor.x, Sun->DiffuseColor.y, Sun->DiffuseColor.z };
    static float SunAmbientColor[3] = { Sun->AmbientColor.x, Sun->AmbientColor.y, Sun->AmbientColor.z };
    static float SunSpecularColor[3] = { Sun->SpecularColor.x, Sun->SpecularColor.y, Sun->SpecularColor.z };
    static float SunDirection[3] = { Sun->GetRotation().x, Sun->GetRotation().y, Sun->GetRotation().z };

    if (ImGui::CollapsingHeader("Directional"))
    {
		ImGui::ColorEdit3("Diffuse", SunDiffuseColor);
		Sun->DiffuseColor = XMFLOAT4(SunDiffuseColor[0], SunDiffuseColor[1], SunDiffuseColor[2], 1.0f);

		ImGui::ColorEdit3("Ambient", SunAmbientColor);
		Sun->AmbientColor = XMFLOAT4(SunAmbientColor[0], SunAmbientColor[1], SunAmbientColor[2], 1.0f);

		ImGui::ColorEdit3("Specular", SunSpecularColor);
		Sun->SpecularColor = XMFLOAT4(SunSpecularColor[0], SunSpecularColor[1], SunSpecularColor[2], 1.0f);

		ImGui::SliderFloat3("Direction", SunDirection, -180.0f, 180.0f);
		Sun->SetRotation(XMFLOAT3(SunDirection[0], SunDirection[1], SunDirection[2]));
    }

	Mesh* CurrentMesh = Meshes[0];
	static float Rotation[3] = { CurrentMesh->GetRotation().x, CurrentMesh->GetRotation().y, CurrentMesh->GetRotation().z };

	ImGui::Text("Rotation : %f, %f, %f", CurrentMesh->GetRotation().x, CurrentMesh->GetRotation().y, CurrentMesh->GetRotation().z);
	ImGui::SliderFloat3("Rotation", Rotation, -180.0f, 180.0f);
	CurrentMesh->SetRotation(XMFLOAT3(Rotation[0], Rotation[1], Rotation[2]));

	static float Position[3] = { CurrentMesh->GetPosition().x, CurrentMesh->GetPosition().y, CurrentMesh->GetPosition().z };

	ImGui::SliderFloat3("Position", Position, -180.0f, 180.0f);
	CurrentMesh->SetPosition(XMFLOAT3(Position[0], Position[1], Position[2]));

    ImGui::Text("View");
    if (ImGui::Button("Lit"))
        CurrentPixelShader = PixelShader;
    ImGui::SameLine();
    if (ImGui::Button("Unlit"))
        CurrentPixelShader = UnlitPixelShader;
    ImGui::SameLine();
    if (ImGui::Button("Normal"))
        CurrentPixelShader = NormalPixelShader;

	if (ImGui::Button("Toggle Light Emitters"))
		bDrawLightEmitters = !bDrawLightEmitters;
        
    ImGui::Text("FrameTime %.3f ms/frame (%.1f FPS)", FrameTime, 1000.0 / FrameTime);

    ImGui::End();
}

// Helper method to clear the back buffers.
void Renderer::Clear()
{
    // Clear the views.
    D3dContext->ClearRenderTargetView(RenderTargetView.Get(), Colors::Aqua);
    D3dContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    D3dContext->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), DepthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(OutputWidth), static_cast<float>(OutputHeight));
    D3dContext->RSSetViewports(1, &viewport);

    D3dContext->RSSetState(SolidState);
    D3dContext->OMSetDepthStencilState(DepthStencilState.Get(), 0);
}

// Presents the back buffer contents to the screen.
void Renderer::Present()
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
void Renderer::OnActivated()
{
    // TODO: Renderer is becoming active window.
}

void Renderer::OnDeactivated()
{
    // TODO: Renderer is becoming background window.
}

void Renderer::OnSuspending()
{
    // TODO: Renderer is being power-suspended (or minimized).
}

void Renderer::OnResuming()
{
    Timer.ResetElapsedTime();

    // TODO: Renderer is being power-resumed (or returning from minimize).
}

void Renderer::OnWindowSizeChanged(int width, int height)
{
    OutputWidth = std::max(width, 1);
    OutputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Renderer window is being resized.
}

// Properties
void Renderer::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1920;
    height = 1080;
}

void Renderer::LoadNewModel(std::wstring Path)
{
    SceneCamera->SetPosition(XMVectorSet(0.0f, 5.0f, -7.0f, 0.0f));

    // load a mesh  
    wchar_t Dir[_MAX_DIR];
    wchar_t Dump[_MAX_PATH];

    _wsplitpath_s(Path.c_str(), Dump, Dir, Dump, Dump);
    Meshes.clear();

	Assimp::Importer Importer;

	const aiScene* Scene = Importer.ReadFile(DX::WStringToString(Path), aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
        aiProcess_FlipUVs |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

    if (Scene)
    {
        // Extract the models
        aiNode* Node = Scene->mRootNode;
        ParseAssimpNode(Node, Scene, Dir);

        if (Sun)
        {
            delete Sun;
            Sun = nullptr;
        }
        Lights.clear();

        // Extract the lights
        for (unsigned int i = 0; i < Scene->mNumLights; ++i)
        {
            aiLight* CurrentLight = Scene->mLights[i];

            if (CurrentLight)
            {
				XMFLOAT3 Position = XMFLOAT3(CurrentLight->mPosition.x, CurrentLight->mPosition.y, CurrentLight->mPosition.z);
				XMFLOAT4 Ambient = XMFLOAT4(CurrentLight->mColorAmbient.r, CurrentLight->mColorAmbient.g, CurrentLight->mColorAmbient.b, 1.0f);
				XMFLOAT4 Diffuse = XMFLOAT4(CurrentLight->mColorDiffuse.r, CurrentLight->mColorDiffuse.g, CurrentLight->mColorDiffuse.b, 1.0f);
				XMFLOAT4 Specular = XMFLOAT4(CurrentLight->mColorSpecular.r, CurrentLight->mColorSpecular.g, CurrentLight->mColorSpecular.b, 1.0f);

				if (CurrentLight->mType == aiLightSource_DIRECTIONAL)
				{
					XMFLOAT3 Direction = XMFLOAT3(CurrentLight->mDirection.x, CurrentLight->mDirection.y, CurrentLight->mDirection.z);

					DirectionalLight* Directional = new DirectionalLight(Position, Ambient, Diffuse, Specular, Direction);
                    // We add some ambient as we do not have GI for now
                    if (Directional->AmbientColor.x == 0.0f && Directional->AmbientColor.y == 0.0f && Directional->AmbientColor.z == 0.0f);
                    {
                        Directional->AmbientColor = XMFLOAT4(.1f, .1f, .1f, 1.0f);
                    }

					Sun = Directional;
				}

				if (CurrentLight->mType == aiLightSource_POINT)
				{
					//XMFLOAT3 Attenuation = XMFLOAT3(CurrentLight->mAttenuationConstant, CurrentLight->mAttenuationLinear, CurrentLight->mAttenuationQuadratic);

					//PointLight* Point = new PointLight(Position, Ambient, Diffuse, Specular, Attenuation);
					//Lights.push_back(Point);
				}
            }		
        }

		if (!Sun)
		{
			Sun = new DirectionalLight(XMFLOAT3(0.8, -0.1, -0.6), XMFLOAT4(.1f, .1f, .1f, 1.0f), XMFLOAT4(1.f, 1.f, 1.f, 1.0f), XMFLOAT4(1.f, 1.f, 1.f, 1.0f), XMFLOAT3(0.8, -0.1, -0.6));
		}

        AddPointLight(XMFLOAT3(-100.0, 100.0, 0.0), XMFLOAT4(1.f, 0.f, 0.f, 1.0f), XMFLOAT4(1.f, 0.f, 0.f, 1.0f));

        AddPointLight(XMFLOAT3(300.0, 100.0, 0.0), XMFLOAT4(0.f, 0.f, 1.f, 1.0f), XMFLOAT4(0.f, 0.f, 1.f, 1.0f));		

    }
}

void Renderer::AddPointLight(XMFLOAT3 Position, XMFLOAT4 DiffuseColor, XMFLOAT4 SpecularColor)
{
    LightAndMesh NewLightStruct;
	PointLight* NewLight = new PointLight(Position, XMFLOAT4(0.f, 0.f, 0.f, 1.0f), DiffuseColor, SpecularColor, XMFLOAT3(1.0, 0.0014, 0.000007));
    Mesh* LightMesh = new Cube(NewLight->GetPosition(), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(5.0f, 5.0f, 5.0f));

    NewLightStruct.Light = NewLight;
    NewLightStruct.LightMesh = LightMesh;

	Lights.push_back(NewLightStruct);
}

void Renderer::ParseAssimpNode(aiNode* Node, const aiScene* Scene, wchar_t* Dir)
{
	for (unsigned int i = 0; i < Node->mNumMeshes; ++i)
	{
		aiMesh* CurrentMesh = Scene->mMeshes[Node->mMeshes[i]];

		Mesh* NewMesh = new Mesh(CurrentMesh, Node, Scene, std::wstring(Dir));

		NewMesh->InitMesh(D3dDevice, D3dContext);
		Meshes.push_back(NewMesh);
	}

    for (unsigned int i = 0; i < Node->mNumChildren; ++i)
    {
        ParseAssimpNode(Node->mChildren[i], Scene, Dir);
    }
}

// These are the resources that depend on the device.
void Renderer::CreateDevice()
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
    //Font = std::make_unique<SpriteFont>(D3dDevice.Get(), L"Assets/Fonts/Font.spriteFont");
    Batch = std::make_unique<SpriteBatch>(D3dContext.Get());

    // Initialize the camera
    SceneCamera = new Camera();

    // load a mesh
    LoadNewModel(L"Assets/Models/Shapes/TestScene.fbx");

    // Compile the shaders
	VertexShader = new Shader(L"Shaders/SimpleVertexShader.hlsl", EShaderType::VertexShader, device);
	PixelShader = new Shader(L"Shaders/SimplePixelShader.hlsl", EShaderType::PixelShader, device);
    UnlitPixelShader = new Shader(L"Shaders/UnlitPixelShader.hlsl", EShaderType::PixelShader, device);
    NormalPixelShader = new Shader(L"Shaders/NormalPixelShader.hlsl", EShaderType::PixelShader, device);

    CurrentPixelShader = PixelShader;

	// Create and set the InputLayout
	D3D11_INPUT_ELEMENT_DESC Layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT LayoutNum = ARRAYSIZE(Layout);
	DX::ThrowIfFailed(device->CreateInputLayout(Layout, LayoutNum, VertexShader->ShaderBuffer->GetBufferPointer(), VertexShader->ShaderBuffer->GetBufferSize(), InputLayout.GetAddressOf()));

    // Create the lights
    //Lights.push_back(Light);

}

// Allocate all memory resources that change on a window SizeChanged event.
void Renderer::CreateResources()
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
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

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
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 2, 0, D3D11_BIND_DEPTH_STENCIL);
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.MipLevels = 1;

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(D3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS);
    DX::ThrowIfFailed(D3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, DepthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.
    SceneCamera->UpdateProjectionMatrix(OutputWidth, OutputHeight);

    // Constant buffer
    D3D11_BUFFER_DESC ConstantBufferDescriptor;
    ZeroMemory(&ConstantBufferDescriptor, sizeof(D3D11_BUFFER_DESC));
    ConstantBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
    ConstantBufferDescriptor.ByteWidth = sizeof(PerObjectBuffStruct_VS);
    ConstantBufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ConstantBufferDescriptor.CPUAccessFlags = 0;
    ConstantBufferDescriptor.MiscFlags = 0;
    DX::ThrowIfFailed(D3dDevice->CreateBuffer(&ConstantBufferDescriptor, nullptr, PerObjectBuffer_VS.GetAddressOf()));

	ZeroMemory(&ConstantBufferDescriptor, sizeof(D3D11_BUFFER_DESC));
	ConstantBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
	ConstantBufferDescriptor.ByteWidth = sizeof(PerFrameBuffStruct_PS);
	ConstantBufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ConstantBufferDescriptor.CPUAccessFlags = 0;
	ConstantBufferDescriptor.MiscFlags = 0;
    DX::ThrowIfFailed(D3dDevice->CreateBuffer(&ConstantBufferDescriptor, nullptr, PerFrameBuffer_PS.GetAddressOf()));

	ZeroMemory(&ConstantBufferDescriptor, sizeof(D3D11_BUFFER_DESC));
	ConstantBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
	ConstantBufferDescriptor.ByteWidth = sizeof(PerObjectBuffStruct_PS);
	ConstantBufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ConstantBufferDescriptor.CPUAccessFlags = 0;
	ConstantBufferDescriptor.MiscFlags = 0;
	DX::ThrowIfFailed(D3dDevice->CreateBuffer(&ConstantBufferDescriptor, nullptr, PerObjectBuffer_PS.GetAddressOf()));

    FontPos.x = backBufferWidth / 2.0f;
    FontPos.y = backBufferHeight / 2.0f;

    // Rasterizer States
    D3D11_RASTERIZER_DESC RasterizerDesc;

    ZeroMemory(&RasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
    RasterizerDesc.FillMode = D3D11_FILL_SOLID;
    RasterizerDesc.CullMode = D3D11_CULL_NONE;
    RasterizerDesc.MultisampleEnable = true;
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

void Renderer::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    for (auto Mesh : Meshes)
    {
        delete Mesh;
    }
    delete Sun;
    Lights.clear();

	delete VertexShader;
	delete PixelShader;
	delete UnlitPixelShader;
	delete NormalPixelShader;

    InputLayout->Release();
    DepthStencilView.Reset();
    RenderTargetView.Reset();

    PerObjectBuffer_VS->Release();
    PerFrameBuffer_PS->Release();
    SolidState->Release();
    DepthStencilState->Release();
    WireFrameState->Release();
    SwapChain.Reset();
    D3dContext.Reset();
    D3dDevice.Reset();
    Font.reset();
    Batch.reset();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

    CreateDevice();

    CreateResources();
}
