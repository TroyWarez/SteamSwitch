#pragma once
#include "framework.h"
#include "MonitorHandler.h"

#define STEAM_DESK L"Steam"
#define SDL_CLASS L"SDL_app"
#define ICUE_CLASS L"Qt672QWindowIcon"
#define ICUE_TITLE L"iCUE"
#define MOUSE_WAKETIME 50000000

class SteamHandler
{
public:
	SteamHandler(HWND hWnd);
	~SteamHandler();
	HWND mainHwnd;
	HMODULE hKernel32;
	IUIAutomation* pAutomation;
	int StartSteamHandler();
	int getSteamPid();
	LPCWSTR getSteamBigPictureModeTitle() { return steamBigPictureModeTitle.c_str(); }
	bool isSteamRunning();
	bool isSteamInGame();
	bool SetSteamFocus();
	MonitorHandler* monHandler;
	IUIAutomationElement* BPwindow;
	LONG(WINAPI* NtQueryInformationProcess)(HANDLE ProcessHandle, ULONG ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
	DWORD steamPid;
	DWORD gamePid;
	std::wstring steamBigPictureModeTitle;
	bool isSteamInBigPictureMode;
	bool ShouldRefocus;
	void ShouldFocus(bool focus);
};

