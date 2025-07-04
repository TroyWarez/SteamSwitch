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
	LPCWSTR getSteamBigPictureModeTitle() { return steamBigPictureModeTitle.c_str(); }
	bool isSteamRunning();
	bool isSteamInGame();
	MonitorHandler* monHandler;
	LONG(WINAPI* NtQueryInformationProcess)(HANDLE ProcessHandle, ULONG ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
	DWORD steamPid;
	DWORD gamePid;
	std::wstring steamBigPictureModeTitle;
	bool isSteamInBigPictureMode;
};

