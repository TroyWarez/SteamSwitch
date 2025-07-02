#pragma once
#include "framework.h"
class AudioHandler
{
public:
	AudioHandler();
	~AudioHandler();
	int setDefaultAudioDevice(int device);
	int getDefaultAudioDevice(int device);
	void ToggleMode();
	void InitDefaultAudioDevice(LPCWSTR Device_FriendlyName);
};

