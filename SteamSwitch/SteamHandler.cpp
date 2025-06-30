#include "SteamHandler.h"
#define STEAM_DESK L"Steam"
#define STEAM_DESK_CLASS L"SDL_app"
#define MOUSE_WAKETIME 50000000
#include "Settings.h"
// bool IsSteamInBigPictureMode();
//void onState(void* context, const CorsairSessionStateChanged* eventData)
//{
	//return;
//}
SteamHandler::SteamHandler()
{
	//CorsairError er = CorsairConnect(onState, NULL);
	//CorsairError er2 = CorsairRequestControl(NULL, CAL_ExclusiveLightingControlAndKeyEventsListening);
	WCHAR windowsDir[MAX_PATH] = { 0 };
	std::wstring windowsPath(windowsDir);
	std::wstring windowsExplorerPath(windowsDir);
	if (GetWindowsDirectoryW(windowsDir, MAX_PATH))
	{
		windowsPath = windowsDir;
		windowsExplorerPath = windowsPath + L"\\explorer.exe";
		windowsPath = windowsPath + L"\\Cursors\\";
	}
	steamPid = getSteamPid();
	monHandler = new MonitorHandler(MonitorHandler::DESK_MODE);
	audioHandler = new AudioHandler();
	DWORD PID = 0;
	LARGE_INTEGER ticks = { 0 };
	LARGE_INTEGER ticks2 = { 2 };
	POINT firstCursorPos = { 0 };
	while (true)
	{
		if (isSteamRunning())
		{
			HWND hWnd = FindWindowW(STEAM_DESK_CLASS, STEAM_DESK);
			if (hWnd == NULL)
			{
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
						HWND eH = FindWindowW(L"Progman", NULL);
						GetWindowThreadProcessId(eH, &PID);
						PostMessage(eH, /*WM_QUIT*/ 0x12, 0, 0);
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
						DestroyCursor(h);
						//CorsairError er = CorsairRequestControl(NULL, CAL_ExclusiveLightingControlAndKeyEventsListening);
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

										HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, PID);
										TerminateProcess(hProcess, 0);
										CloseHandle(hProcess);
										ShellExecuteW(NULL, L"open", windowsExplorerPath.c_str(), NULL, NULL, SW_SHOW);

										std::wstring cursorFileName = windowsPath + L"aero_arrow.cur";
										BOOL ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_NORMAL);

										cursorFileName = windowsPath + L"beam_i.cur";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_IBEAM);

										cursorFileName = windowsPath + L"aero_working.ani";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_WAIT);

										cursorFileName = windowsPath + L"cross_i.cur";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_CROSS);

										cursorFileName = windowsPath + L"aero_up_l.cur";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_UP);

										cursorFileName = windowsPath + L"aero_nwse.cur";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_SIZENWSE);

										cursorFileName = windowsPath + L"aero_nesw.cur";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_SIZENESW);

										cursorFileName = windowsPath + L"aero_ew.cur";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_SIZEWE);

										cursorFileName = windowsPath + L"aero_ns.cur";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_SIZENS);

										cursorFileName = windowsPath + L"aero_move.cur";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_SIZEALL);

										cursorFileName = windowsPath + L"no_i.cur";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_NO);

										cursorFileName = windowsPath + L"aero_link.cur";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_HAND);

										cursorFileName = windowsPath + L"aero_working.ani";
										ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_APPSTARTING);


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
									DestroyCursor(h);
									//Dangerous, but it works.
									SetCursorPos((GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1), (GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1));
								}
								else if (cursorPos.x == firstCursorPos.x && cursorPos.y == firstCursorPos.y && ticks.QuadPart == 0 &&
									cursorPos.x != (GetSystemMetrics(SM_CXVIRTUALSCREEN) / 2) && cursorPos.y != (GetSystemMetrics(SM_CYVIRTUALSCREEN) / 2))
								{
									QueryPerformanceCounter(&ticks);
									ticks.QuadPart += MOUSE_WAKETIME;

									std::wstring cursorFileName = windowsPath + L"aero_arrow_l.cur";
									BOOL ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_NORMAL);

									cursorFileName = windowsPath + L"beam_il.cur";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_IBEAM);

									cursorFileName = windowsPath + L"aero_working_l.ani";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_WAIT);

									cursorFileName = windowsPath + L"cross_i.cur";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_CROSS);

									cursorFileName = windowsPath + L"aero_up_l.cur";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_UP);

									cursorFileName = windowsPath + L"aero_nwse.cur";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_SIZENWSE);

									cursorFileName = windowsPath + L"aero_nesw.cur";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_SIZENESW);

									cursorFileName = windowsPath + L"aero_ew.cur";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_SIZEWE);

									cursorFileName = windowsPath + L"aero_ns.cur";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_SIZENS);

									cursorFileName = windowsPath + L"aero_move.cur";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_SIZEALL);

									cursorFileName = windowsPath + L"no_i.cur";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_NO);

									cursorFileName = windowsPath + L"aero_link.cur";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_HAND);

									cursorFileName = windowsPath + L"aero_working_l.ani";
									ret = SetSystemCursor(CopyCursor(LoadCursorFromFileW(cursorFileName.c_str())), OCR_APPSTARTING);
								}
								else if (cursorPos.x != firstCursorPos.x && cursorPos.y != firstCursorPos.y)
								{
									ticks = { 0 };
								}
							}
							GetCursorPos(&firstCursorPos);
							QueryPerformanceCounter(&ticks2);
							if (ticks.QuadPart != 0 && ticks2.QuadPart >= ticks.QuadPart)
							{
								if (cursorPos.x != (GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1) && cursorPos.y != (GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1))
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
									DestroyCursor(h);

									//Dangerous, but it works.
									SetCursorPos((GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1), (GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1));
								}
								ticks = { 0 };
								ticks2 = { 2 };
							}
						}
					}
				Sleep(1);
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