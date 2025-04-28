#include "WasapiAudioRenderer.h"

#include <Windows.h>
#include <wrl\implements.h>
#include <mmreg.h>
#include <mfapi.h>
#include "string"

//using namespace Microsoft::WRL;
//using namespace Windows::Media::Devices;
//using namespace Windows::Storage::Streams;

void WasapiAudioRenderer::Initialize()
{
	//IActivateAudioInterfaceAsyncOperation* asyncOp;
	//HRESULT Hr = S_OK;

	//// Get a string representing the Default Audio Device Renderer
	//std::string m_DeviceIdString = MediaDevice::GetDefaultAudioRenderId(Windows::Media::Devices::AudioDeviceRole::Default);

	//// This call must be made on the main UI thread.  Async operation will call back to 
	// // IActivateAudioInterfaceCompletionHandler::ActivateCompleted, which must be an agile interface implementation
	//Hr = ActivateAudioInterfaceAsync(m_DeviceIdString->Data(), __uuidof(IAudioClient3), nullptr, this, &asyncOp);
	//if (FAILED(Hr))
	//{
	//	m_DeviceStateChanged->SetState(DeviceState::DeviceStateInError, hr, true);
	//}

	////SAFE_RELEASE(asyncOp);

	//return Hr;
}

void WasapiAudioRenderer::Tick()
{
	
}
