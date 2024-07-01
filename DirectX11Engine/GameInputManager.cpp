#include "Core/pch.h"
#include "Core/Renderer.h"
#include "Camera.h"
#include "Mesh/Mesh.h"

#include "GameInputManager.h"

using namespace DirectX;

extern void ExitGame() noexcept;

void GameInputManager::Initialize(HWND Window, Renderer* InOwner)
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
		Owner->SceneCamera->Speed = 1.0f;
	}

	HandleMouse();

	if (KeyboardState.Up || KeyboardState.Z)
	{
		Zoom(1);
	}
	if (KeyboardState.Down || KeyboardState.S)
	{
		Zoom(-1);
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
	if (LastMouseState.scrollWheelValue != LastFrameWheelValue)
	{
		Zoom(LastMouseState.scrollWheelValue - LastFrameWheelValue);
		LastFrameWheelValue = LastMouseState.scrollWheelValue;
	}
	if (KeyboardState.NumPad1)
	{
		Owner->SceneCamera->Speed -= .05;
		if (Owner->SceneCamera->Speed <= 0)
		{
			Owner->SceneCamera->Speed = .1;
		}
	}
	if (KeyboardState.NumPad3)
	{
		Owner->SceneCamera->Speed += .1;
		if (Owner->SceneCamera->Speed >= 50)
		{
			Owner->SceneCamera->Speed = 50;
		}
	}
	if (KeyboardState.N)
	{
		Owner->CurrentPixelShader = Owner->NormalPixelShader;
	}
	if (KeyboardState.U)
	{
		Owner->CurrentPixelShader = Owner->UnlitPixelShader;
	}
	if (KeyboardState.L)
	{
		Owner->CurrentPixelShader = Owner->PixelShader;
	}
}

void GameInputManager::HandleMouse()
{
	DirectX::Mouse::State MouseState = Mouse->GetState();
	if (MouseState.positionMode == Mouse::MODE_RELATIVE)
	{
		if (MouseState.x != LastX)
		{
			RotateYaw((LastX + MouseState.x) * 0.015);
		}
		if (MouseState.y != LastY)
		{
			RotatePitch((LastY + MouseState.y) * 0.015);
		}

		LastX = Mouse->GetState().x;
		LastY = Mouse->GetState().y;
	}

	Mouse->SetMode(MouseState.rightButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
}

void GameInputManager::Zoom(int ZoomValue)
{
	float Value = ZoomValue * Owner->SceneCamera->Speed;
	Owner->SceneCamera->SetPosition(Owner->SceneCamera->GetPosition() + Owner->SceneCamera->GetForwardVector() * Value);
}

void GameInputManager::MoveRight(int RightValue)
{
	Owner->SceneCamera->SetPosition(Owner->SceneCamera->GetPosition() + Owner->SceneCamera->GetRightVector() * (RightValue * Owner->SceneCamera->Speed));
}

void GameInputManager::MoveUp(int UpValue)
{
	float Value = UpValue * Owner->SceneCamera->Speed;
	Owner->SceneCamera->SetPosition(Owner->SceneCamera->GetPosition() + Owner->SceneCamera->GetUpVector() * Value);
	Owner->SceneCamera->SetTarget(Owner->SceneCamera->GetTarget() + Owner->SceneCamera->GetUpVector() * Value);
}

void GameInputManager::RotateYaw(float YawValue)
{
	float Value = YawValue;
	Owner->SceneCamera->SetForwardVector(XMVector3Transform(Owner->SceneCamera->GetForwardVector(), XMMatrixRotationAxis(Owner->SceneCamera->GetUpVector(), Value)));
}

void GameInputManager::RotatePitch(float PitchValue)
{
	float Value = PitchValue;
	Owner->SceneCamera->SetForwardVector(XMVector3Transform(Owner->SceneCamera->GetForwardVector(), XMMatrixRotationAxis(Owner->SceneCamera->GetRightVector(), Value)));
}

