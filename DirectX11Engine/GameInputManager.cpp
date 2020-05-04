#include "pch.h"
#include "Game.h"
#include "Camera.h"

#include "GameInputManager.h"

using namespace DirectX;

extern void ExitGame() noexcept;

void GameInputManager::Initialize(HWND Window, Game* InOwner)
{
	// Input 
	Keyboard = std::make_unique<DirectX::Keyboard>();
	Mouse = std::make_unique<DirectX::Mouse>();
	Mouse->SetWindow(Window);

	this->Owner = InOwner;
}

void GameInputManager::Update()
{
	// Input
	auto KeyboardState = Keyboard->GetState();
	if (KeyboardState.Escape)
	{
		ExitGame();
	}
	if (KeyboardState.Home)
	{
		Owner->SceneCamera->SetPosition(XMVectorSet(0.0f, 0.0f, -7.0f, 0.0f));
	}

	auto MouseState = Mouse->GetState();


	if (KeyboardState.Up || KeyboardState.Z)
	{
		Zoom(10);
	}
	if (KeyboardState.Down || KeyboardState.S)
	{
		Zoom(-10);
	}
	if (KeyboardState.Left || KeyboardState.Q)
	{
		MoveRight(-1);
	}
	if (KeyboardState.Right || KeyboardState.D)
	{
		MoveRight(+1);
	}
	if (KeyboardState.PageUp || KeyboardState.Space)
	{
		MoveUp(1);
	}
	if (KeyboardState.PageDown || KeyboardState.X)
	{
		MoveUp(-1);
	}
	if (MouseState.scrollWheelValue != LastFrameWheelValue)
	{
		Zoom(MouseState.scrollWheelValue - LastFrameWheelValue);
		LastFrameWheelValue = MouseState.scrollWheelValue;
	}
}

void GameInputManager::Zoom(int ZoomValue)
{
	Owner->SceneCamera->SetPosition(Owner->SceneCamera->GetPosition() + Owner->SceneCamera->GetForwardVector() * (float)ZoomValue / 100);
	Owner->SceneCamera->SetTarget(Owner->SceneCamera->GetTarget() + Owner->SceneCamera->GetForwardVector() * (float)ZoomValue / 100);
}

void GameInputManager::MoveRight(int RightValue)
{
	Owner->SceneCamera->SetPosition(Owner->SceneCamera->GetPosition() + Owner->SceneCamera->GetRightVector() * (float)RightValue / 10);
	Owner->SceneCamera->SetTarget(Owner->SceneCamera->GetTarget() + Owner->SceneCamera->GetRightVector() * (float)RightValue / 10);
}

void GameInputManager::MoveUp(int UpValue)
{
	Owner->SceneCamera->SetPosition(Owner->SceneCamera->GetPosition() + Owner->SceneCamera->GetUpVector() * (float)UpValue / 10);
	Owner->SceneCamera->SetTarget(Owner->SceneCamera->GetTarget() + Owner->SceneCamera->GetUpVector() * (float)UpValue / 10);
}

