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
	HCURSOR CursorH1 = LoadCursor(NULL, IDC_ARROW);
	HCURSOR CursorH2 = LoadCursor(NULL, IDC_IBEAM);
	HCURSOR CursorH3 = LoadCursor(NULL, IDC_WAIT);
	HCURSOR CursorH4 = LoadCursor(NULL, IDC_CROSS);
	HCURSOR CursorH5 = LoadCursor(NULL, IDC_UPARROW);
	HCURSOR CursorH6 = LoadCursor(NULL, IDC_SIZE);
	HCURSOR CursorH7 = LoadCursor(NULL, IDC_ICON);
	HCURSOR CursorH8 = LoadCursor(NULL, IDC_SIZENWSE);
	HCURSOR CursorH9 = LoadCursor(NULL, IDC_SIZEWE);
	HCURSOR CursorH10 = LoadCursor(NULL, IDC_SIZEWE);
	HCURSOR CursorH11 = LoadCursor(NULL, IDC_SIZENS);
	HCURSOR CursorH12 = LoadCursor(NULL, IDC_SIZEALL);
	HCURSOR CursorH13 = LoadCursor(NULL, IDC_NO);
	HCURSOR CursorH14 = LoadCursor(NULL, IDC_HAND);
	HCURSOR CursorH15 = LoadCursor(NULL, IDC_APPSTARTING);
	HCURSOR CursorH16 = LoadCursor(NULL, IDC_HELP);
	HCURSOR CursorH17 = LoadCursor(NULL, IDC_PIN);
	HCURSOR CursorH18 = LoadCursor(NULL, IDC_PERSON);

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
						HCURSOR h = LoadCursorFromFileW(L"invisible-cursor.cur");
						BOOL ret = SetSystemCursor(CopyCursor(h), OCR_NORMAL);
						ret = SetSystemCursor(CopyCursor(h), OCR_IBEAM);
						ret = SetSystemCursor(CopyCursor(h), OCR_WAIT);
						ret = SetSystemCursor(CopyCursor(h), OCR_CROSS);
						ret = SetSystemCursor(CopyCursor(h), OCR_UP);
						ret = SetSystemCursor(CopyCursor(h), OCR_SIZENWSE);
						ret = SetSystemCursor(CopyCursor(h), OCR_SIZENESW);
						ret = SetSystemCursor(CopyCursor(h), OCR_SIZEWE);
						ret = SetSystemCursor(CopyCursor(h), OCR_SIZENS);
						ret = SetSystemCursor(CopyCursor(h), OCR_SIZEALL);
						ret = SetSystemCursor(CopyCursor(h), OCR_NO);
						ret = SetSystemCursor(CopyCursor(h), OCR_HAND);
						ret = SetSystemCursor(CopyCursor(h), OCR_APPSTARTING);
						monHandler->ToggleMode();
						Sleep(20);
						audioHandler->InitDefaultAudioDevice(defaultBpAudioDevice);
						isSteamInBigPictureMode = true;
						while (isSteamInBigPictureMode)
						{
							if (isSteamRunning())
							{
								HWND hWndBP = FindWindowW(STEAM_DESK_CLASS, title.c_str());
									if (hWndBP == NULL) {
										BOOL ret = SetSystemCursor(CursorH1, OCR_NORMAL);
										ret = SetSystemCursor(CursorH2, OCR_IBEAM);
										ret = SetSystemCursor(CursorH3, OCR_WAIT);
										ret = SetSystemCursor(CursorH4, OCR_CROSS);
										ret = SetSystemCursor(CursorH5, OCR_UP);
										ret = SetSystemCursor(CursorH7, OCR_SIZENWSE);
										ret = SetSystemCursor(CursorH8, OCR_SIZENESW);
										ret = SetSystemCursor(CursorH9, OCR_SIZEWE);
										ret = SetSystemCursor(CursorH11, OCR_SIZENS);
										ret = SetSystemCursor(CursorH12, OCR_SIZEALL);
										ret = SetSystemCursor(CursorH13, OCR_NO);
										ret = SetSystemCursor(CursorH14, OCR_HAND);
										ret = SetSystemCursor(CursorH15, OCR_APPSTARTING);
										monHandler->ToggleMode();
										audioHandler->InitDefaultAudioDevice(defaultDeskAudioDevice);
										isSteamInBigPictureMode = false;
										break;
									}
							}
							POINT cursorPos;
							if (GetCursorPos(&cursorPos))
							{
								if (cursorPos.x == (GetSystemMetrics(SM_CXVIRTUALSCREEN) / 2) && cursorPos.y == (GetSystemMetrics(SM_CYVIRTUALSCREEN) / 2))
								{
									SetCursorPos((GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1), (GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1));
								}
							}
							Sleep(1);
						}
					}
				}
				Sleep(1);
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