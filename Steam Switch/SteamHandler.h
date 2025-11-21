#pragma once
#include "framework.h"
#include "MonitorHandler.h"
#include "InputHandler.h"
#include "SerialHandler.h"
#include "AudioHandler.h"

constexpr LPCWSTR STEAM_DESK = L"Steam";
constexpr LPCWSTR SDL_CLASS = L"SDL_app";
constexpr LPCWSTR ICUE_CLASS = L"Qt672QWindowIcon";
constexpr LPCWSTR ICUE_TITLE = L"iCUE";
constexpr int MOUSE_WAKETIME = 50000000;
constexpr int CONTROLLER_WAKETIME = 20000000;
DWORD WINAPI ICUEThread(LPVOID lpParam);
class SteamHandler
{
public:
	SteamHandler(HWND hWnd);
	~SteamHandler();
	SerialHandler serialHandler;
	HWND mainHwnd;
	HMODULE hKernel32;
	int StartSteamHandler();
	int getSteamPid();
	const LPCWSTR getSteamBigPictureModeTitle() { return steamBigPictureModeTitle.c_str(); }
	bool isSteamRunning();
	bool isSteamInGame();
	const bool getSteamFocus();
	MonitorHandler* monHandler;
	InputHandler* inputHandler;
	AudioHandler* audioHandler;
	LONG(WINAPI* NtQueryInformationProcess)(HANDLE ProcessHandle, ULONG ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
	DWORD steamPid;
	DWORD gamePid;
	std::wstring steamBigPictureModeTitle;
	bool isSteamInBigPictureMode;
	bool isSteamFocused;
	bool isIcueInstalled;
	HANDLE hShutdownEvent;
	HANDLE hBPEvent;
	HANDLE hFindIcueEvent;
	HANDLE hCloseIcueEvent;

	HANDLE hICUEThread;
	HANDLE hCECThread;
	HANDLE hSerialThread;
};

