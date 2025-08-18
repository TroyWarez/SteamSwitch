#pragma once
#include "framework.h"
class AudioHandler
{
public:
	AudioHandler();
	~AudioHandler();
	std::wstring BPaudioDeviceName;
	std::wstring DESKaudioDeviceName;
	void setDefaultAudioDevice(LPCWSTR newDevice);
	LPCWSTR getDefaultAudioDevice();
	bool BPisDefaultAudioDevice();
	void InitDefaultAudioDevice();
};

