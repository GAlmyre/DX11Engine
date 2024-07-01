#pragma once
#include "Core/pch.h"

class GameInputManager
{
public:
	void Initialize(HWND Window, class Renderer* Owner);

	void Update();

	void HandleMouse();

	Renderer* Owner;

	DirectX::Mouse::State LastMouseState;

	std::unique_ptr<DirectX::Keyboard> Keyboard;
	std::unique_ptr<DirectX::Mouse> Mouse;

private:

	bool bFirstFrame = true;

	int LastX, LastY;
	
	int LastFrameWheelValue = 0;

	void Zoom(int ZoomValue);

	void MoveRight(int RightValue);

	void MoveUp(int UpValue);

	void RotateYaw(float YawValue);

	void RotatePitch(float PitchValue);
};

