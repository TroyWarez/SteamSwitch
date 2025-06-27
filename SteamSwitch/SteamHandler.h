#pragma once
#define OEMRESOURCE
#include <windows.h>
#include <iostream>
#include "iCUESDK.h"
#include "MonitorHandler.h"
#include "AudioHandler.h"
class SteamHandler
{
public:
	SteamHandler();
	~SteamHandler();
	int getSteamPid();
	bool isSteamRunning();
	MonitorHandler* monHandler;
	AudioHandler* audioHandler;
	DWORD steamPid;
	bool isSteamInBigPictureMode;
};

