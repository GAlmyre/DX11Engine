#pragma once

// Base class for all audio renderers
class AudioRenderer
{
public:
	virtual void Initialize() = 0;
	virtual void Tick() = 0;
};

