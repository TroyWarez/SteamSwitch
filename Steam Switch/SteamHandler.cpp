#include "SteamHandler.h"
#define STEAM_DESK L"Steam"
#define SDL_CLASS L"SDL_app"
#define ICUE_CLASS L"Qt672QWindowIcon"
#define ICUE_TITLE L"iCUE"
#define MOUSE_WAKETIME 50000000
#include "Settings.h"
#include "InvisibleMouse.h"
SteamHandler::SteamHandler()
{
	steamPid = getSteamPid();
	gamePid = 0;
	monHandler = new MonitorHandler(MonitorHandler::DESK_MODE);
	HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
	if (hKernel32)
	{
		*(FARPROC*)&NtQueryInformationProcess = GetProcAddress(hKernel32, "NtQueryInformationProcess");
	}
} 
bool SteamHandler::isSteamInGame()
{
	steamPid = getSteamPid();
	std::vector <std::wstring> processNames;
	ULONG_PTR pbi[6];
	ULONG ulSize = 0;
	HMODULE hMods[1024] = { 0 };
	DWORD cbNeeded = 0;
	std::vector<DWORD> processIds(1024);
	if (EnumProcesses(processIds.data(), ((DWORD)processIds.size() * sizeof(DWORD)), &cbNeeded))
	{
			for (size_t i = 0; i < processIds.size(); i++)
			{
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);
				if (hProcess)
				{
					WCHAR szProcessName[MAX_PATH] = { 0 };
					if (GetModuleBaseNameW(hProcess, hMods[i], szProcessName, sizeof(szProcessName) / sizeof(WCHAR)))
					{
						processNames.push_back(szProcessName);

						if (NtQueryInformationProcess) {
							if (NtQueryInformationProcess(hProcess, 0, &pbi, sizeof(pbi), &ulSize) >= 0 && ulSize == sizeof(pbi))
							{
								std::wstring processName(szProcessName);

								if (pbi[5] == steamPid && processName != L"steamwebhelper.exe")
								{
									gamePid = processIds[i];
									return true;
								}
							}
						}
						CloseHandle(hProcess);
					}
				}
			}
	}
	return false;
}
int SteamHandler::StartSteamHandler()
{
	//bool isSteamInGameBool = isSteamInGame();
	bool TopMost = false;
	DWORD er = GetLastError();
	bool ShouldRightClick = true;
	WCHAR windowsDir[MAX_PATH] = { 0 };
	std::wstring windowsPath(windowsDir);
	std::wstring windowsExplorerPath(windowsDir);
	if (GetWindowsDirectoryW(windowsDir, MAX_PATH))
	{
		windowsPath = windowsDir;
		windowsExplorerPath = windowsPath + L"\\explorer.exe";
		windowsPath = windowsPath + L"\\Cursors\\";
	}
	DWORD PID = 0;
	LARGE_INTEGER ticks = { 0 };
	LARGE_INTEGER ticks2 = { 2 };

	LARGE_INTEGER xticks = { 0 };
	LARGE_INTEGER xticks2 = { 2 };

	bool ShouldHideCursor = true;
	bool ButtonPressed = false;
	bool SelectButtonPressed = false;
	XINPUT_STATE xstate = { 0 };
	POINT firstCursorPos = { 0 };
	MSG msg;
	WCHAR programFiles[MAX_PATH] = { 0 };
	ExpandEnvironmentStringsW(L"%PROGRAMFILES%", programFiles, MAX_PATH);
	std::wstring programFilesPath(programFiles);
	programFilesPath = programFilesPath + L"\\Corsair\\Corsair iCUE5 Software\\iCUE Launcher.exe";

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (isSteamRunning())
			{
				HWND hWnd = FindWindowW(SDL_CLASS, STEAM_DESK);
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

					if (subtitle == STEAM_DESK && classname == SDL_CLASS && title != subtitle)
					{
						steamBigPictureModeTitle = title;
						//HWND bpHwnd = FindWindowW(SDL_CLASS, title.c_str());
						//if (bpHwnd){
							//SetWindowPos(bpHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
						//}

						//HWND eH = FindWindowW(L"Progman", NULL);
						//GetWindowThreadProcessId(eH, &PID);
						//PostMessage(eH, /*WM_QUIT*/ 0x12, 0, 0);

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
						monHandler->ToggleMode();
						Sleep(20);
						isSteamInBigPictureMode = true;

						while (isSteamInBigPictureMode)
						{
							if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
							{
								if (msg.message == WM_QUIT)
								{
									break;
								}

								TranslateMessage(&msg);
								DispatchMessage(&msg);
							}
							else
							{
								if (isSteamRunning())
								{
									HWND hWndBP = FindWindowW(SDL_CLASS, title.c_str());
									if (hWndBP == NULL) {
										//HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, PID);
										//TerminateProcess(hProcess, 0);
										//CloseHandle(hProcess);

										std::wstring cursorFileName = windowsPath + L"aero_arrow.cur";
										BOOL ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_NORMAL);

										cursorFileName = windowsPath + L"beam_i.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_IBEAM);

										cursorFileName = windowsPath + L"aero_working.ani";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_WAIT);

										cursorFileName = windowsPath + L"cross_i.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_CROSS);

										cursorFileName = windowsPath + L"aero_up_l.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_UP);

										cursorFileName = windowsPath + L"aero_nwse.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_SIZENWSE);

										cursorFileName = windowsPath + L"aero_nesw.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_SIZENESW);

										cursorFileName = windowsPath + L"aero_ew.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_SIZEWE);

										cursorFileName = windowsPath + L"aero_ns.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_SIZENS);

										cursorFileName = windowsPath + L"aero_move.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_SIZEALL);

										cursorFileName = windowsPath + L"no_i.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_NO);

										cursorFileName = windowsPath + L"aero_link.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_HAND);

										cursorFileName = windowsPath + L"aero_working.ani";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_APPSTARTING);


										monHandler->ToggleMode();
										isSteamInBigPictureMode = false;
										break;
									}
									else
									{
										HWND foreHwnd = GetForegroundWindow();

										WCHAR windowTitle[256] = { 0 };
										GetWindowTextW(foreHwnd, windowTitle, 256);
										std::wstring title2(windowTitle);
										WCHAR windowClassName[256] = { 0 };
										GetClassNameW(foreHwnd, windowClassName, 256);
										std::wstring classname(windowClassName);

										HWND icueHwnd = FindWindowW(ICUE_CLASS, ICUE_TITLE);

										if (icueHwnd)
										{
											ShowWindow(icueHwnd, SW_HIDE);
										}

										//if (classname != SDL_CLASS && title2 != STEAM_DESK && !isSteamInGame())
										//{
											//SetActiveWindow(icueHwnd);
											//SwitchToThisWindow(icueHwnd, TRUE);
											//SetWindowPos(hWndBP, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
										//}
									}
								}
								DWORD dwResult = XInputGetState(0, &xstate); 
								if (dwResult == ERROR_SUCCESS)
								{

									if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
										xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT &&
										xticks.QuadPart == 0 && !ButtonPressed)
									{
										QueryPerformanceCounter(&xticks);
										xticks.QuadPart += MOUSE_WAKETIME / 2;
									}
									else if (
										xticks.QuadPart <= xticks2.QuadPart &&
										xticks2.QuadPart != 2 &&
										xticks.QuadPart != 0)
									{
										(ShouldHideCursor) ? ShouldHideCursor = false : ShouldHideCursor = true;
										xticks = { 0 };
										xticks2 = { 2 };
										ButtonPressed = true;
									}

									else if (!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ||
										!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT))
									{
										xticks = { 0 };
										xticks2 = { 2 };
										ButtonPressed = false;
									}
									if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
									{
										if (!SelectButtonPressed)
										{
											if (!isSteamInGame())
											{
												HWND hWndBP2 = FindWindowW(SDL_CLASS, title.c_str());
												ShowWindow(hWndBP2, SW_SHOW);
												SetActiveWindow(hWndBP2);
												SetForegroundWindow(hWndBP2);
												SwitchToThisWindow(hWndBP2, TRUE);
											}
											SelectButtonPressed = true;
										}
									}
									else
									{
										SelectButtonPressed = false;
									}
									QueryPerformanceCounter(&xticks2);
								}
								POINT cursorPos;
								if (GetCursorPos(&cursorPos))
								{
									if (cursorPos.x == (GetSystemMetrics(SM_CXVIRTUALSCREEN) / 2) && cursorPos.y == (GetSystemMetrics(SM_CYVIRTUALSCREEN) / 2))
									{
										HCURSOR h = CreateIconFromResourceEx((PBYTE)&InvisCursorData, sizeof(InvisCursorData), TRUE, 0x00030000, 0, 0, LR_DEFAULTSIZE | LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_SHARED);
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
										if (ShouldHideCursor)
										{
											SetCursorPos((GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1), (GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1));
										}
									}
									else if (cursorPos.x == firstCursorPos.x && cursorPos.y == firstCursorPos.y && ticks.QuadPart == 0 &&
										cursorPos.x != (GetSystemMetrics(SM_CXVIRTUALSCREEN) / 2) && cursorPos.y != (GetSystemMetrics(SM_CYVIRTUALSCREEN) / 2))
									{
										QueryPerformanceCounter(&ticks);
										ticks.QuadPart += MOUSE_WAKETIME;

										std::wstring cursorFileName = windowsPath + L"aero_arrow_l.cur";
										BOOL ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_NORMAL);

										cursorFileName = windowsPath + L"beam_il.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_IBEAM);

										cursorFileName = windowsPath + L"aero_working_l.ani";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_WAIT);

										cursorFileName = windowsPath + L"cross_i.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_CROSS);

										cursorFileName = windowsPath + L"aero_up_l.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_UP);

										cursorFileName = windowsPath + L"aero_nwse.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_SIZENWSE);

										cursorFileName = windowsPath + L"aero_nesw.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_SIZENESW);

										cursorFileName = windowsPath + L"aero_ew.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_SIZEWE);

										cursorFileName = windowsPath + L"aero_ns.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_SIZENS);

										cursorFileName = windowsPath + L"aero_move.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_SIZEALL);

										cursorFileName = windowsPath + L"no_i.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_NO);

										cursorFileName = windowsPath + L"aero_link.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_HAND);

										cursorFileName = windowsPath + L"aero_working_l.ani";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_APPSTARTING);
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

										if (ShouldHideCursor) {
											SetCursorPos((GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1), (GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1));
										}
									}
									ticks = { 0 };
									ticks2 = { 2 };
								}
							}
							Sleep(1);
						}
					}
				}
			}
		}
		Sleep(1);
	}
	return (int)msg.wParam;
}
SteamHandler::~SteamHandler()
{
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
	steamPid = getSteamPid();
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