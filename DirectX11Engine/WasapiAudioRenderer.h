#pragma once

// Audio renderer using Windows' WASAPI

#include "AudioRenderer.h"

#include <Windows.h>
#include <wrl\implements.h>
#include <mfapi.h>
#include <AudioClient.h>
#include <mmdeviceapi.h>

class WasapiAudioRenderer :
    public AudioRenderer
{
public:

    void Initialize() override;
    void Tick() override;

private:

};

