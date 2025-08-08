#pragma once
#include "framework.h"
#include "MonitorHandler.h"
#include "InputHandler.h"

#define STEAM_DESK L"Steam"
#define SDL_CLASS L"SDL_app"
#define ICUE_CLASS L"Qt672QWindowIcon"
#define ICUE_TITLE L"iCUE"
#define MOUSE_WAKETIME 50000000
#define CONTROLLER_WAKETIME 20000000
static HANDLE iCueThreadHandle = NULL;
DWORD WINAPI ICUEThread(LPVOID lpParam);
class SteamHandler
{
public:
	SteamHandler(HWND hWnd);
	~SteamHandler();
	HWND mainHwnd;
	HMODULE hKernel32;
	HANDLE hSafeToRestoreEvent;
	int StartSteamHandler();
	int getSteamPid();
	LPCWSTR getSteamBigPictureModeTitle() { return steamBigPictureModeTitle.c_str(); }
	bool isSteamRunning();
	bool isSteamInGame();
	bool getSteamFocus();
	MonitorHandler* monHandler;
	InputHandler* inputHandler;
	LONG(WINAPI* NtQueryInformationProcess)(HANDLE ProcessHandle, ULONG ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
	DWORD steamPid;
	DWORD gamePid;
	std::wstring steamBigPictureModeTitle;
	bool isSteamInBigPictureMode;
	bool isSteamFocused;
};

