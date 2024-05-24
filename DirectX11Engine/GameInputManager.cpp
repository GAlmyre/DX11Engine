#include "Core/pch.h"
#include "Core/Renderer.h"
#include "Camera.h"

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

	auto MouseState = Mouse->GetState();


	if (KeyboardState.Up || KeyboardState.Z)
	{
		Zoom(1);
	}
	if (KeyboardState.Down || KeyboardState.S)
	{
		Zoom(-1);
	}
	if (KeyboardState.A)
	{
		RotateYaw(-.025);
	}
	if (KeyboardState.E)
	{
		RotateYaw(.025);
	}

	if (KeyboardState.W)
	{
		RotatePitch(-.025);
	}
	if (KeyboardState.C)
	{
		RotatePitch(.025);
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

	if (KeyboardState.NumPad8)
	{
		Owner->Sun->Direction = XMFLOAT3(Owner->Sun->Direction.x, Owner->Sun->Direction.y, Owner->Sun->Direction.z +0.01);
	}
	if (KeyboardState.NumPad2)
	{
		Owner->Sun->Direction = XMFLOAT3(Owner->Sun->Direction.x, Owner->Sun->Direction.y, Owner->Sun->Direction.z - 0.01);
	}
	if (KeyboardState.NumPad4)
	{
		Owner->Sun->Direction = XMFLOAT3(Owner->Sun->Direction.x, Owner->Sun->Direction.y + 0.01, Owner->Sun->Direction.z);
	}
	if (KeyboardState.NumPad6)
	{
		Owner->Sun->Direction = XMFLOAT3(Owner->Sun->Direction.x, Owner->Sun->Direction.y - 0.01, Owner->Sun->Direction.z);
	}
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

