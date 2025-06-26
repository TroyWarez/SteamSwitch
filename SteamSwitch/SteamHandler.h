#pragma once
#include <windows.h>
class SteamHandler
{
public:
	SteamHandler();
	~SteamHandler();
	int getSteamPid();
	DWORD steamPid;
};

