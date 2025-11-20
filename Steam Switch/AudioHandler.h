#pragma once
#include "framework.h"
class AudioHandler
{
public:
	AudioHandler();
	~AudioHandler();
	std::wstring BPaudioDeviceName;
	std::wstring lastBPaudioDeviceName;
	std::wstring DESKaudioDeviceName;
	IMMDeviceEnumerator* pEnum;
	IPolicyConfigVista* pPolicyConfig;
	void setDefaultAudioDevice(LPCWSTR newDevice);
	const LPCWSTR getDefaultAudioDevice();
	const bool BPisDefaultAudioDevice();
	void InitDefaultAudioDevice();
	void ToggleAudioDevice();
	HRESULT SetDefaultAudioPlaybackDevice(LPCWSTR devID);
};

