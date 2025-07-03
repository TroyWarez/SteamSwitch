#pragma once
#include "framework.h"
#include "MonitorHandler.h"
class SteamHandler
{
public:
	SteamHandler();
	~SteamHandler();
	int StartSteamHandler();
	int getSteamPid();
	bool isSteamRunning();
	MonitorHandler* monHandler;
	DWORD steamPid;
	bool isSteamInBigPictureMode;
};

