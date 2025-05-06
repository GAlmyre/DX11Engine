//
// Renderer.cpp
//

#include "Core/pch.h"
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
#include <ShObjIdl_core.h>
#include <thread>
#include <array>

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Renderer::Renderer() noexcept :
	Window(nullptr),
	OutputWidth(2048),
	OutputHeight(1152),
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
	
	Timer.SetFixedTimeStep(true);
	Timer.SetTargetElapsedSeconds(1.0 / 60);
	

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
		InputManager->Update();       
	});
	Update(Timer);

	Render();
}

// Updates the world.
void Renderer::Update(DX::StepTimer const& timer)
{
	float elapsedTime = float(timer.GetElapsedSeconds());

	FrameTime = elapsedTime;
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

	// Blending
	float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT SampleMask = 0xffffffff;
	D3dContext->OMSetBlendState(BlendState.Get(), BlendFactor, SampleMask);

	// Set Input layout
	
	D3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> VSRef = VertexShader->GetVertexShaderRef();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> PSRef = CurrentPixelShader->GetPixelShaderRef();

	D3dContext->VSSetShader(VSRef.Get(), 0, 0);
	D3dContext->PSSetShader(PSRef.Get(), 0, 0);

	// LightingPass

	// Draw each mesh of the scene
	switch (RenderingType)
	{
	case ERenderingType::Lit:
		DrawLit();
		break;
	case ERenderingType::Unlit:
		DrawUnlit();
		break;
	case ERenderingType::Normal:
		DrawNormals(false);
		break;
	case ERenderingType::NormalWithMaps:
		DrawNormals(true);
		break;
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	Present();
}

void Renderer::DrawLit()
{
	//SceneCamera->SetPosition(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));

	PerFrameBuffStruct_PS.Sun = Sun->GetLightData();
	if (!bToggleDirectional)
	{
		PerFrameBuffStruct_PS.Sun.AmbientColor = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		PerFrameBuffStruct_PS.Sun.DiffuseColor = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		PerFrameBuffStruct_PS.Sun.SpecularColor = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		PerFrameBuffStruct_PS.Sun.Position = Sun->GetPosition();
	}

	for (int i = 0; i < MAX_LIGHTS && i < (int)Lights.size(); ++i)
	{
		PerFrameBuffStruct_PS.PointLights[i] = Lights[i]->Light->GetLightData();
	}

	XMFLOAT3 CamPos{};
	XMStoreFloat3(&CamPos, SceneCamera->GetPosition());
	PerFrameBuffStruct_PS.CameraPosition = CamPos;
	PerFrameBuffStruct_PS.LightsCount = (int)Lights.size();

	//RenderShadowDepth();

	D3dContext->IASetInputLayout(LitInputLayout.Get());

	D3dContext->PSSetShader(PixelShader->GetPixelShaderRef().Get(), nullptr, 0);
	D3dContext->VSSetShader(VertexShader->GetVertexShaderRef().Get(), nullptr, 0);

	D3dContext.Get()->PSSetShaderResources(3, 1, ShadowMapSRV.GetAddressOf());
	D3dContext.Get()->PSSetSamplers(1, 1, &ShadowMapSamplerState);

	for (Mesh* Mesh : Meshes)
	{
		WorldViewProj = Mesh->GetWorldMatrix() * SceneCamera->GetViewMatrix() * SceneCamera->GetProjectionMatrix();
		LightWorldViewProj = Mesh->GetWorldMatrix() * Sun->GetViewMatrix() * Sun->GetProjectionMatrix(OutputWidth, OutputHeight);

		PerObjectBuffStruct_VS.WorldViewProj = XMMatrixTranspose(WorldViewProj);
		PerObjectBuffStruct_VS.World = XMMatrixTranspose(Mesh->GetWorldMatrix());
		PerObjectBuffStruct_VS.LightWorldViewProj = XMMatrixTranspose(LightWorldViewProj);
		//PerObjectBuffStruct_VS.LightPos = XMFLOAT4(Sun->GetPosition().x, Sun->GetPosition().y, Sun->GetPosition().z, 1.0f);

		PerObjectBuffStruct_PS.Mat = Mesh->Material;

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
		for (LightAndMesh* CurrentLight : Lights)
		{
			D3dContext->PSSetShader(UnlitPixelShader->GetPixelShaderRef().Get(), 0, 0);

			WorldViewProj = CurrentLight->LightMesh->GetWorldMatrix() * SceneCamera->GetViewMatrix() * SceneCamera->GetProjectionMatrix();
			PerObjectBuffStruct_VS.WorldViewProj = XMMatrixTranspose(WorldViewProj);
			PerObjectBuffStruct_VS.World = XMMatrixTranspose(CurrentLight->LightMesh->GetWorldMatrix());

			CurrentLight->LightMesh->InitMesh(D3dDevice, D3dContext);

			PerObjectBuffStruct_PS.Mat = CurrentLight->LightMesh->Material;

			D3dContext->UpdateSubresource(PerObjectBuffer_PS.Get(), 0, nullptr, &PerObjectBuffStruct_PS, 0, 0);
			D3dContext->PSSetConstantBuffers(0, 1, PerObjectBuffer_PS.GetAddressOf());

			D3dContext->UpdateSubresource(PerObjectBuffer_VS.Get(), 0, nullptr, &PerObjectBuffStruct_VS, 0, 0);
			D3dContext->VSSetConstantBuffers(0, 1, PerObjectBuffer_VS.GetAddressOf());

			CurrentLight->LightMesh->Draw(D3dContext);
		}
	}

}

void Renderer::RenderShadowDepth()
{
	// Shadow map
	D3dContext->OMSetRenderTargets(1, ShadowMapRTV.GetAddressOf(), DepthStencilView.Get());
	D3dContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	D3dContext->IASetInputLayout(ShadowDepthInputLayout.Get());

	Microsoft::WRL::ComPtr<ID3D11VertexShader> VSRef = ShadowMapVS->GetVertexShaderRef();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> PSRef = ShadowMapPS->GetPixelShaderRef();

	D3dContext->VSSetShader(VSRef.Get(), nullptr, 0);
	D3dContext->PSSetShader(PSRef.Get(), nullptr, 0);

	for (Mesh* Mesh : Meshes)
	{
		WorldViewProj = Mesh->GetWorldMatrix() * Sun->GetViewMatrix() * Sun->GetProjectionMatrix(OutputWidth, OutputHeight);

		PerObjectBuffStruct_VS.WorldViewProj = XMMatrixTranspose(WorldViewProj);
		PerObjectBuffStruct_VS.World = XMMatrixTranspose(Mesh->GetWorldMatrix());
		PerObjectBuffStruct_VS.LightWorldViewProj = XMMatrixTranspose(WorldViewProj);

		D3dContext->UpdateSubresource(PerObjectBuffer_VS.Get(), 0, nullptr, &PerObjectBuffStruct_VS, 0, 0);
		D3dContext->VSSetConstantBuffers(0, 1, PerObjectBuffer_VS.GetAddressOf());

		Mesh->Draw(D3dContext);
	}

	D3dContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	D3dContext->ClearRenderTargetView(RenderTargetView.Get(), Colors::DarkGray);
	D3dContext->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), DepthStencilView.Get());
}

void Renderer::DrawUnlit()
{
	D3dContext->IASetInputLayout(LitInputLayout.Get());

	for (Mesh* Mesh : Meshes)
	{
		WorldViewProj = Mesh->GetWorldMatrix() * SceneCamera->GetViewMatrix() * SceneCamera->GetProjectionMatrix();

		PerObjectBuffStruct_VS.WorldViewProj = XMMatrixTranspose(WorldViewProj);
		PerObjectBuffStruct_VS.World = XMMatrixTranspose(Mesh->GetWorldMatrix());

		PerObjectBuffStruct_PS.Mat = Mesh->Material;

		D3dContext->UpdateSubresource(PerObjectBuffer_PS.Get(), 0, nullptr, &PerObjectBuffStruct_PS, 0, 0);
		D3dContext->PSSetConstantBuffers(0, 1, PerObjectBuffer_PS.GetAddressOf());

		D3dContext->UpdateSubresource(PerObjectBuffer_VS.Get(), 0, nullptr, &PerObjectBuffStruct_VS, 0, 0);
		D3dContext->VSSetConstantBuffers(0, 1, PerObjectBuffer_VS.GetAddressOf());

		Mesh->Draw(D3dContext);
	}
	if (bDrawLightEmitters)
	{
		// Draw meshes for the lights
		for (LightAndMesh* CurrentLight : Lights)
		{
			D3dContext->PSSetShader(UnlitPixelShader->GetPixelShaderRef().Get(), nullptr, 0);

			WorldViewProj = CurrentLight->LightMesh->GetWorldMatrix() * SceneCamera->GetViewMatrix() * SceneCamera->GetProjectionMatrix();
			PerObjectBuffStruct_VS.WorldViewProj = XMMatrixTranspose(WorldViewProj);
			PerObjectBuffStruct_VS.World = XMMatrixTranspose(CurrentLight->LightMesh->GetWorldMatrix());

			CurrentLight->LightMesh->InitMesh(D3dDevice, D3dContext);

			PerObjectBuffStruct_PS.Mat = CurrentLight->LightMesh->Material;

			D3dContext->UpdateSubresource(PerObjectBuffer_PS.Get(), 0, nullptr, &PerObjectBuffStruct_PS, 0, 0);
			D3dContext->PSSetConstantBuffers(0, 1, PerObjectBuffer_PS.GetAddressOf());

			D3dContext->UpdateSubresource(PerObjectBuffer_VS.Get(), 0, nullptr, &PerObjectBuffStruct_VS, 0, 0);
			D3dContext->VSSetConstantBuffers(0, 1, PerObjectBuffer_VS.GetAddressOf());

			CurrentLight->LightMesh->Draw(D3dContext);
		}
	}
}

void Renderer::DrawNormals(bool bWithMaps)
{
	D3dContext->IASetInputLayout(LitInputLayout.Get());
	for (Mesh* Mesh : Meshes)
	{
		WorldViewProj = Mesh->GetWorldMatrix() * SceneCamera->GetViewMatrix() * SceneCamera->GetProjectionMatrix();

		PerObjectBuffStruct_VS.WorldViewProj = XMMatrixTranspose(WorldViewProj);
		PerObjectBuffStruct_VS.World = XMMatrixTranspose(Mesh->GetWorldMatrix());

		PerObjectBuffStruct_PS.Mat = Mesh->Material;
		if (!bWithMaps)
		{
			PerObjectBuffStruct_PS.Mat.bUseNormalMap = false;
		}

		D3dContext->UpdateSubresource(PerObjectBuffer_PS.Get(), 0, nullptr, &PerObjectBuffStruct_PS, 0, 0);
		D3dContext->PSSetConstantBuffers(0, 1, PerObjectBuffer_PS.GetAddressOf());

		D3dContext->UpdateSubresource(PerObjectBuffer_VS.Get(), 0, nullptr, &PerObjectBuffStruct_VS, 0, 0);
		D3dContext->VSSetConstantBuffers(0, 1, PerObjectBuffer_VS.GetAddressOf());

		Mesh->Draw(D3dContext);
	}
	if (bDrawLightEmitters)
	{
		// Draw meshes for the lights
		for (LightAndMesh* CurrentLight : Lights)
		{
			D3dContext->PSSetShader(UnlitPixelShader->GetPixelShaderRef().Get(), 0, 0);

			WorldViewProj = CurrentLight->LightMesh->GetWorldMatrix() * SceneCamera->GetViewMatrix() * SceneCamera->GetProjectionMatrix();
			PerObjectBuffStruct_VS.WorldViewProj = XMMatrixTranspose(WorldViewProj);
			PerObjectBuffStruct_VS.World = XMMatrixTranspose(CurrentLight->LightMesh->GetWorldMatrix());

			CurrentLight->LightMesh->InitMesh(D3dDevice, D3dContext);

			PerObjectBuffStruct_PS.Mat = CurrentLight->LightMesh->Material;

			D3dContext->UpdateSubresource(PerObjectBuffer_PS.Get(), 0, nullptr, &PerObjectBuffStruct_PS, 0, 0);
			D3dContext->PSSetConstantBuffers(0, 1, PerObjectBuffer_PS.GetAddressOf());

			D3dContext->UpdateSubresource(PerObjectBuffer_VS.Get(), 0, nullptr, &PerObjectBuffStruct_VS, 0, 0);
			D3dContext->VSSetConstantBuffers(0, 1, PerObjectBuffer_VS.GetAddressOf());

			CurrentLight->LightMesh->Draw(D3dContext);
		}
	}
}

void Renderer::DrawGui()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	

	ImGui::Begin("Settings");
	if (ImGui::Button("Open"))
		OpenModel();

	ImGui::SliderFloat("Camera Speed", &SceneCamera->Speed, 0.1f, 50.0f);

	if (Sun)
	{
		float SunDiffuseColor[3] = { Sun->DiffuseColor.x, Sun->DiffuseColor.y, Sun->DiffuseColor.z };

		float SunAmbientColor[3] = { Sun->AmbientColor.x, Sun->AmbientColor.y, Sun->AmbientColor.z };
		float SunSpecularColor[3] = { Sun->SpecularColor.x, Sun->SpecularColor.y, Sun->SpecularColor.z };
		float SunDirection[3] = { Sun->GetRotation().x, Sun->GetRotation().y, Sun->GetRotation().z };
		float SunPosition[3] = { Sun->GetPosition().x, Sun->GetPosition().y, Sun->GetPosition().z };


		if (ImGui::CollapsingHeader("Directional"))
		{
			if (ImGui::Button("Toggle Directional"))
				bToggleDirectional = !bToggleDirectional;

			if (ImGui::ColorEdit3("Diffuse", SunDiffuseColor))
				Sun->DiffuseColor = XMFLOAT4(SunDiffuseColor[0], SunDiffuseColor[1], SunDiffuseColor[2], 1.0f);

			if (ImGui::ColorEdit3("Ambient", SunAmbientColor))
				Sun->AmbientColor = XMFLOAT4(SunAmbientColor[0], SunAmbientColor[1], SunAmbientColor[2], 1.0f);

			if (ImGui::ColorEdit3("Specular", SunSpecularColor))
				Sun->SpecularColor = XMFLOAT4(SunSpecularColor[0], SunSpecularColor[1], SunSpecularColor[2], 1.0f);

			if (ImGui::SliderFloat3("Direction", SunDirection, -180.0f, 180.0f))
				Sun->SetRotation(XMFLOAT3(SunDirection[0], SunDirection[1], SunDirection[2]));

			if (ImGui::SliderFloat3("Position", SunPosition, -10000.0f, 10000.0f))
				Sun->SetPosition(XMFLOAT3(SunPosition[0], SunPosition[1], SunPosition[2]));
		}
	}

	// Handle the point lights
	std::vector< std::array<float, 3>> PointLightsLocations;
	for (LightAndMesh* Light : Lights)
	{
		std::array<float, 3> NewLocation = { Light->Light->GetPosition().x, Light->Light->GetPosition().y, Light->Light->GetPosition().z };
		PointLightsLocations.push_back(NewLocation);
	}

	if (ImGui::CollapsingHeader("Point Lights"))
	{
		int LightIndex = 0;
		for (LightAndMesh* Light : Lights)
		{
			std::string CategoryName = Light->Light->GetName();
			if (ImGui::SliderFloat3(CategoryName.c_str(), &PointLightsLocations[LightIndex][0], -1500.0f, 1500.0f))
				Light->SetPosition(XMFLOAT3(PointLightsLocations[LightIndex][0], PointLightsLocations[LightIndex][1], PointLightsLocations[LightIndex][2]));
			LightIndex++;
		}
	}

	if (ImGui::Button("Toggle Light Emitters"))
	{
		bDrawLightEmitters = !bDrawLightEmitters;
	}

	// Handle the Meshes
	std::vector< std::array<float, 3>> MeshesLocations;
	for (Mesh* CurrentMesh : Meshes)
	{
		std::array<float, 3> NewLocation = { CurrentMesh->GetPosition().x, CurrentMesh->GetPosition().y, CurrentMesh->GetPosition().z };
		MeshesLocations.push_back(NewLocation);
	}

	if (ImGui::CollapsingHeader("Meshes"))
	{
		int MeshIndex = 0;
		for (Mesh* CurrentMesh : Meshes)
		{
			std::string CategoryName = CurrentMesh->GetName();
			if (ImGui::InputFloat3(CategoryName.c_str(), &MeshesLocations[MeshIndex][0]))
				CurrentMesh->SetPosition(XMFLOAT3(MeshesLocations[MeshIndex][0], MeshesLocations[MeshIndex][1], MeshesLocations[MeshIndex][2]));
			MeshIndex++;
		}
	}

	if (ImGui::CollapsingHeader("View modes", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("View");
		if (ImGui::Button("Lit"))
		{
			CurrentPixelShader = PixelShader;
			RenderingType = ERenderingType::Lit;
		}

		ImGui::SameLine();
		if (ImGui::Button("Unlit"))
		{
			CurrentPixelShader = UnlitPixelShader;
			RenderingType = ERenderingType::Unlit;
		}

		ImGui::SameLine();
		if (ImGui::Button("Normal"))
		{
			CurrentPixelShader = NormalPixelShader;
			RenderingType = ERenderingType::Normal;
		}

		ImGui::SameLine();
		if (ImGui::Button("Normal with maps"))
		{
			CurrentPixelShader = NormalPixelShader;
			RenderingType = ERenderingType::NormalWithMaps;
		}
	}
			 
	//ImGui::ShowDemoWindow();

	ImGui::Text("FrameTime %.3f ms/frame (%.1f FPS)", FrameTime, 1000.0 / FrameTime);

	ImGui::End();
}

// Helper method to clear the back buffers.
void Renderer::Clear()
{
	// Clear the views.
	D3dContext->ClearRenderTargetView(RenderTargetView.Get(), Colors::DarkGray);
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
	width = 2048;
	height = 1152;
}

void Renderer::LoadNewModel(std::wstring Path)
{
	Logger::Log(std::string("Opening model : ") + DX::WStringToString(Path));

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
		Lights.clear();
		if (Sun)
		{
			delete Sun;
			Sun = nullptr;
		}

		ParseAssimpNode(Node, Scene, Dir);

		if (!Sun)
		{
			Sun = new DirectionalLight(XMFLOAT3(50.0, 150.0, -800.0), XMFLOAT4(.1f, .1f, .1f, 1.0f), XMFLOAT4(.8f, .8f, .8f, 1.0f), XMFLOAT4(.8f, .8f, .8f, 1.0f), XMFLOAT3(7., 5., 0.));
		}		

		Logger::Log("Sun up vector : " + std::to_string(Sun->GetUpVector().x) + ", " + std::to_string(Sun->GetUpVector().y) + ", " + std::to_string(Sun->GetUpVector().z), LogSeverity::Warning);
	}
	else
	{
		Logger::Log("Scene Failed to load", LogSeverity::Error);
	}
}

void Renderer::AddPointLight(XMFLOAT3 Position, XMFLOAT4 DiffuseColor, XMFLOAT4 SpecularColor, XMFLOAT3 Attenuation, std::string Name)
{
	LightAndMesh* NewLightStruct = new LightAndMesh();
	// We override attenuation as assimp doesn't get it right from fbx files
	PointLight* NewLight = new PointLight(Position, XMFLOAT4(0.f, 0.f, 0.f, 1.0f), DiffuseColor, SpecularColor, XMFLOAT3(1.0, 0.0014, 0.000007));
	//PointLight* NewLight = new PointLight(Position, XMFLOAT4(0.f, 0.f, 0.f, 1.0f), DiffuseColor, SpecularColor, Attenuation);
	Mesh* LightMesh = new Cube(NewLight->GetPosition(), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(5.0f, 5.0f, 5.0f));

	NewLightStruct->Light = NewLight;
	NewLightStruct->LightMesh = LightMesh;
	NewLightStruct->Light->SetName(Name);
	NewLightStruct->LightMesh->SetName(Name + "_mesh");

	Lights.push_back(NewLightStruct);
}

void Renderer::ParseAssimpNode(aiNode* Node, const aiScene* Scene, wchar_t* Dir)
{
	aiLight* Light = nullptr;
	// Check if this is a light
	for (unsigned int LightIndex = 0; LightIndex < Scene->mNumLights; ++LightIndex)
	{
		if (Scene->mLights[LightIndex]->mName == Node->mName)
		{
			Light = Scene->mLights[LightIndex];
		}
	}

	if (Light)
	{
		aiVector3D NodePosition, NodeScaling, NodeRotation;
		Node->mTransformation.Decompose(NodeScaling, NodeRotation, NodePosition);

		XMFLOAT3 Position = XMFLOAT3(NodePosition.x + Light->mPosition.x, NodePosition.y + Light->mPosition.y, NodePosition.z + Light->mPosition.z);
		XMFLOAT4 Ambient = XMFLOAT4(Light->mColorAmbient.r, Light->mColorAmbient.g, Light->mColorAmbient.b, 1.0f);
		XMFLOAT4 Diffuse = XMFLOAT4(Light->mColorDiffuse.r, Light->mColorDiffuse.g, Light->mColorDiffuse.b, 1.0f);
		XMFLOAT4 Specular = XMFLOAT4(Light->mColorSpecular.r, Light->mColorSpecular.g, Light->mColorSpecular.b, 1.0f);

		if (Light->mType == aiLightSource_POINT)
		{
			XMFLOAT3 Attenuation = XMFLOAT3(Light->mAttenuationConstant, Light->mAttenuationLinear, Light->mAttenuationQuadratic);
			AddPointLight(Position, XMFLOAT4(1.f, 1.f, 1.f, 1.0f), XMFLOAT4(1.f, 1.f, 1.f, 1.0f), Attenuation, Light->mName.C_Str());
		}
	}

	for (unsigned int i = 0; i < Node->mNumMeshes; ++i)
	{
		aiMesh* CurrentMesh = Scene->mMeshes[Node->mMeshes[i]];
		Mesh* NewMesh = new Mesh(CurrentMesh, Node, Scene, std::wstring(Dir));
		NewMesh->InitMesh(D3dDevice, D3dContext);
		NewMesh->SetName(CurrentMesh->mName.C_Str());
		Meshes.push_back(NewMesh);
	}

	for (unsigned int i = 0; i < Node->mNumChildren; i++)
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
	LoadNewModel(L"Assets/Models/ShadowTest/ShadowTest.fbx");

	// Base shaders
	VertexShader = new Shader(L"Shaders/SimpleVertexShader.hlsl", EShaderType::VertexShader, device);
	PixelShader = new Shader(L"Shaders/SimplePixelShader.hlsl", EShaderType::PixelShader, device);

	// Debug view modes
	UnlitPixelShader = new Shader(L"Shaders/UnlitPixelShader.hlsl", EShaderType::PixelShader, device);
	NormalPixelShader = new Shader(L"Shaders/NormalPixelShader.hlsl", EShaderType::PixelShader, device);

	// ShadowMap shaders
	ShadowMapVS = new Shader(L"Shaders/ShadowMapVS.hlsl", EShaderType::VertexShader, device);
	ShadowMapPS = new Shader(L"Shaders/ShadowMapPS.hlsl", EShaderType::PixelShader, device);

	CurrentPixelShader = PixelShader;

	// Create and set the InputLayout
	D3D11_INPUT_ELEMENT_DESC Layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 48,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	DX::ThrowIfFailed(device->CreateInputLayout(Layout, ARRAYSIZE(Layout), VertexShader->ShaderBuffer->GetBufferPointer(), VertexShader->ShaderBuffer->GetBufferSize(), LitInputLayout.GetAddressOf()));
	
	D3D11_INPUT_ELEMENT_DESC ShadowLayout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	
	DX::ThrowIfFailed(device->CreateInputLayout(ShadowLayout, ARRAYSIZE(ShadowLayout), ShadowMapVS->ShaderBuffer->GetBufferPointer(), ShadowMapVS->ShaderBuffer->GetBufferSize(), ShadowDepthInputLayout.GetAddressOf()));
}

// Allocate all memory resources that change on a window SizeChanged event.
void Renderer::CreateResources()
{
	// Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews [] = { nullptr };
	D3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
	RenderTargetView.Reset();
	DepthStencilView.Reset();
	BlendState.Reset();

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
	SceneCamera->UpdateProjectionMatrix((float)OutputWidth, (float)OutputHeight);

	// Set Blend State
	D3D11_BLEND_DESC1 BlendStateDesc;
	ZeroMemory(&BlendStateDesc, sizeof(D3D11_BLEND_DESC1));
	BlendStateDesc.RenderTarget[0].BlendEnable = FALSE;
	BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	DX::ThrowIfFailed(D3dDevice->CreateBlendState1(&BlendStateDesc, BlendState.GetAddressOf()));

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
	RasterizerDesc.FillMode = D3D11_FILL_SOLID;
	RasterizerDesc.CullMode = D3D11_CULL_FRONT;
	RasterizerDesc.MultisampleEnable = true;
	DX::ThrowIfFailed(D3dDevice->CreateRasterizerState(&RasterizerDesc, &ShadowDepthState));

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


	// -------- Shadow mapping test --------
	// texture desc
	CD3D11_TEXTURE2D_DESC ShadowMapDesc = {};
	ShadowMapDesc.Width = backBufferWidth;
	ShadowMapDesc.Height = backBufferHeight;
	ShadowMapDesc.MipLevels = 1;
	ShadowMapDesc.ArraySize = 1;
	ShadowMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ShadowMapDesc.SampleDesc.Count = 1;
	ShadowMapDesc.SampleDesc.Quality = 0;
	ShadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
	ShadowMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ShadowMapDesc.CPUAccessFlags = 0;
	ShadowMapDesc.MiscFlags = 0;

	// depth stencil view desc
	D3D11_RENDER_TARGET_VIEW_DESC ShadowMapRTVDesc = {};
	ShadowMapRTVDesc.Format = ShadowMapDesc.Format;
	ShadowMapRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	ShadowMapRTVDesc.Texture2D.MipSlice = 0;

	// SRV desc
	CD3D11_SHADER_RESOURCE_VIEW_DESC ShadowMapSRVDesc = {};
	ShadowMapSRVDesc.Format = ShadowMapDesc.Format;
	ShadowMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ShadowMapSRVDesc.Texture2D.MipLevels = ShadowMapDesc.MipLevels;
	ShadowMapSRVDesc.Texture2D.MostDetailedMip = 0;

	HRESULT hr;
	hr = D3dDevice->CreateTexture2D(&ShadowMapDesc, nullptr, ShadowMapTexture.GetAddressOf());

	if (FAILED(hr))
	{
		Logger::Log("Failed to create the Texture 2D for shadow map", LogSeverity::Error);
	}
	hr = D3dDevice->CreateRenderTargetView(ShadowMapTexture.Get(), &ShadowMapRTVDesc, ShadowMapRTV.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		Logger::Log("Failed to create the RTV for shadow map", LogSeverity::Error);
	}
	hr = D3dDevice->CreateShaderResourceView(ShadowMapTexture.Get(), &ShadowMapSRVDesc, ShadowMapSRV.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		Logger::Log("Failed to create the SRV for shadow map", LogSeverity::Error);
	}

	D3D11_SAMPLER_DESC SamplerDesc;
	ZeroMemory(&SamplerDesc, sizeof(SamplerDesc));
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.MipLODBias = 0.0f;
	SamplerDesc.MaxAnisotropy = 1;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	SamplerDesc.BorderColor[0] = 0;
	SamplerDesc.BorderColor[1] = 0;
	SamplerDesc.BorderColor[2] = 0;
	SamplerDesc.BorderColor[3] = 0;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = D3dDevice.Get()->CreateSamplerState(&SamplerDesc, ShadowMapSamplerState.GetAddressOf());
	if (FAILED(hr))
	{
		Logger::Log("Failed to create the SRV for shadow map", LogSeverity::Error);
	}

	// ------ End Shadow mapping test ------
}

void Renderer::OnDeviceLost()
{
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
	delete ShadowMapVS;
	delete ShadowMapPS;

	LitInputLayout->Release();
	ShadowDepthInputLayout->Release();
	DepthStencilView.Reset();
	RenderTargetView.Reset();
	BlendState->Release();

	PerObjectBuffer_VS->Release();
	PerFrameBuffer_PS->Release();
	SolidState->Release();
	ShadowDepthState->Release();
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

void Renderer::OpenModel()
{
	HRESULT hr;
	IFileOpenDialog* pFileOpen;

	//COMDLG_FILTERSPEC Filters[1] = { L"ObjExt", L"*.obj" };

	// Create the FileOpenDialog object.
	hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
		IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
	//pFileOpen->SetFileTypes(1, Filters);

	if (SUCCEEDED(hr))
	{
		// Show the Open dialog box.
		hr = pFileOpen->Show(NULL);
		// Get the file name from the dialog box.
		if (SUCCEEDED(hr))
		{
			IShellItem* pItem;
			hr = pFileOpen->GetResult(&pItem);
			if (SUCCEEDED(hr))
			{
				PWSTR Path;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &Path);

				// Display the file name to the user.
				if (SUCCEEDED(hr))
				{
					//LPWSTR CurrentDirStr = new TCHAR[1024];
					//GetCurrentDirectory(1024, CurrentDirStr);
					//std::wstring PathStr = Path;
					//PathStr.erase(0, wcslen(CurrentDirStr) + 1);
					//Path = &PathStr[0];

					LoadNewModel(Path);
				}
				pItem->Release();
			}
		}
	}
}