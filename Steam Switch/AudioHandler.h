#pragma once
#include "framework.h"
class AudioHandler
{
public:
	AudioHandler();
	~AudioHandler();
	std::wstring audioDeviceName;
	void setDefaultAudioDevice(LPCWSTR newDevice);
	LPCWSTR getDefaultAudioDevice();
	void InitDefaultAudioDevice();
};

