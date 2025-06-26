#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "PolicyConfig.h"
#include <iostream>
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

