#pragma once
#include "pch.h"

class GameInputManager
{
public:
	void Initialize(HWND Window, class Game* Owner);

	void Update();

	Game* Owner;

	std::unique_ptr<DirectX::Keyboard> Keyboard;
	std::unique_ptr<DirectX::Mouse> Mouse;

private:
	int LastFrameWheelValue = 0;

	void Zoom(int ZoomValue);

	void MoveRight(int RightValue);

	void MoveUp(int UpValue);

	void RotateYaw(float YawValue);

	void RotatePitch(float PitchValue);
};

