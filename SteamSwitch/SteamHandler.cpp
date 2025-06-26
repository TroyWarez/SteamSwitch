#include "SteamHandler.h"
#define STEAM_DESK L"Steam"
#define STEAM_DESK_CLASS L"SDL_app"
#include "Settings.h"

// bool IsSteamInBigPictureMode();
SteamHandler::SteamHandler()
{
	steamPid = getSteamPid();
	monHandler = new MonitorHandler(MonitorHandler::DESK_MODE);
	audioHandler = new AudioHandler();
	while (true)
	{
		if (isSteamRunning())
		{
			HWND hWnd = FindWindowW(STEAM_DESK_CLASS, STEAM_DESK);
			if (hWnd)
			{
				if (IsWindowVisible(hWnd) == false) {

					HWND foreHwnd = GetForegroundWindow();

					WCHAR windowTitle[256] = { 0 };
					GetWindowTextW(foreHwnd, windowTitle, 256);
					std::wstring title(windowTitle);
					std::wstring subtitle = L"";
					if (title.size() > 5)
					{
						subtitle = title.substr(0, 5);
					}
					WCHAR windowClassName[256] = { 0 };
					GetClassNameW(foreHwnd, windowClassName, 256);
					std::wstring classname(windowClassName);

					if (subtitle == STEAM_DESK && classname == STEAM_DESK_CLASS && title != subtitle)
					{
						monHandler->ToggleMode();
						audioHandler->InitDefaultAudioDevice(defaultBpAudioDevice);
						isSteamInBigPictureMode = true;
						while (isSteamInBigPictureMode)
						{
							if (isSteamRunning())
							{
								HWND hWndBP = FindWindowW(STEAM_DESK_CLASS, title.c_str());
									if (hWndBP == NULL) {
										monHandler->ToggleMode();
										audioHandler->InitDefaultAudioDevice(defaultDeskAudioDevice);
										isSteamInBigPictureMode = false;
										break;
									}
							}
							Sleep(2000);
						}
					}
				}
				Sleep(2000); // Check 60 times every second
			}
			else
			{
				Sleep(3000);
			}
		}
	}
}
SteamHandler::~SteamHandler()
{
	delete audioHandler;
	delete monHandler;
}
int SteamHandler::getSteamPid()
{
	HKEY hKey = NULL;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD type = REG_DWORD;
		DWORD pid = 0;
		DWORD pidSize = sizeof(pid);
		if (RegQueryValueExW(hKey, L"pid", NULL, &type, (BYTE*)&pid, &pidSize) == ERROR_SUCCESS)
		{
			CloseHandle(hKey);
			return pid;
		}
		CloseHandle(hKey);
	}
	return 0;
}
bool SteamHandler::isSteamRunning()
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, steamPid);
	if (hProcess)
	{
		DWORD exitCode = 0;
		if (GetExitCodeProcess(hProcess, &exitCode))
		{
			CloseHandle(hProcess);
			if (exitCode != STILL_ACTIVE)
			{
				std::cout << "Steam process has exited." << std::endl;
				steamPid = getSteamPid();
				return false;
			}
			else
			{
				return true;
			}
		}
		CloseHandle(hProcess);
	}
	return false;
}