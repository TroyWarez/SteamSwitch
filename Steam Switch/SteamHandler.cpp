#include "SteamHandler.h"
#include "InvisibleMouse.h"
SteamHandler::SteamHandler(HWND hWnd)
{
	HINSTANCE hXInputDLL = LoadLibraryA("XInput1_3.dll");
	mainHwnd = hWnd;
	steamPid = getSteamPid();
	gamePid = 0;
	isSteamFocused = true;
	monHandler = new MonitorHandler(MonitorHandler::DESK_MODE);
	inputHandler = new InputHandler();
	hKernel32 = LoadLibraryW(L"NTDLL.DLL");
	if (hKernel32)
	{
		*(FARPROC*)&NtQueryInformationProcess = GetProcAddress(hKernel32, "NtQueryInformationProcess");
	}

}
SteamHandler::~SteamHandler()
{
	if (monHandler)
	{
		delete monHandler;
	}
	if (inputHandler)
	{
		delete inputHandler;
	}
	if (hKernel32)
	{
		FreeLibrary(hKernel32);
		hKernel32 = nullptr;
	}
	CoUninitialize();
}

int SteamHandler::StartSteamHandler()
{
	bool TabTipInvoked = false;
	bool TabTipCordHeld = false;
	bool EnableWindowControls = false;
	bool HeldEnableWindowControls = false;
	bool SelectButtonPressed = false;
	bool guidePressed = false;
	bool TopMost = false;
	bool ShouldHideCursor = true;
	bool ButtonPressed = false;
	bool AButtonPressed = false;
	bool StartButtonPressed = false;
	bool ShouldRightClick = true;

	XINPUT_STATE xstate2 = { 0 };
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
	DWORD icuePid = 0;

	LARGE_INTEGER ticks = { 0 };
	LARGE_INTEGER ticks2 = { 2 };

	LARGE_INTEGER xticks = { 0 };
	LARGE_INTEGER xticks2 = { 2 };

	LARGE_INTEGER ticksGuide = { 0 };
	LARGE_INTEGER ticksGuide2 = { 2 };

	LARGE_INTEGER xticksGuide = { 0 };
	LARGE_INTEGER xticksGuide2 = { 2 };

	XINPUT_STATE xstate = { 0 };
	POINT firstCursorPos = { 0 };
	MSG msg;
	WCHAR programFiles[MAX_PATH] = { 0 };
	ExpandEnvironmentStringsW(L"%PROGRAMFILES%", programFiles, MAX_PATH);
	std::wstring programFilesPath(programFiles);
	programFilesPath = programFilesPath + L"\\Corsair\\Corsair iCUE5 Software\\iCUE Launcher.exe";

	if (monHandler && monHandler->getActiveMonitorCount() == 1)
	{
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
			if (isSteamRunning())
			{
				ShellExecuteW(mainHwnd, L"open", L"steam://open/bigpicture", NULL, NULL, SW_SHOW);
				break;
			}
		}
	}

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
				if (hWnd == NULL)// Don't place anything here.
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
						HWND bpHwnd = FindWindowW(SDL_CLASS, title.c_str());
						SHELLEXECUTEINFOW sei = { sizeof(SHELLEXECUTEINFO) };
						sei.fMask = SEE_MASK_NOCLOSEPROCESS; // Request process handle
						sei.lpFile = programFilesPath.c_str();        // File to execute
						sei.nShow = SW_HIDE;       // How to show the window
						HANDLE hProcessiCue = NULL;

						if (ShellExecuteExW(&sei)) {
							if (sei.hProcess != NULL) {
								hProcessiCue = sei.hProcess;
							}
						}
						steamBigPictureModeTitle = title;

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
						isSteamInBigPictureMode = true;
						ShowWindow(FindWindowW(L"Shell_TrayWnd", NULL), SW_HIDE);
						//SetWindowPos(bpHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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

										inputHandler->turnOffXinputController();
//  										HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, PID);
//  										TerminateProcess(hProcess, 0);
// 										CloseHandle(hProcess);
										HWND hWndIC = FindWindowW(ICUE_CLASS, ICUE_TITLE);
										if (hWndIC)
										{
											PostMessage(hWndIC, /*WM_QUIT*/ 0x12, 0, 0);
										}
//  										PROCESS_INFORMATION pi = { 0 };
// 										STARTUPINFOW si = { 0 };
//  										if (CreateProcessW(windowsExplorerPath.c_str(),
// 											NULL,
//  											NULL,
// 											NULL,
//  											FALSE,
//  											0,
//  											NULL,
//  											NULL,
// 											&si,
//  											&pi
//  										))
//  										{
//  											CloseHandle(pi.hThread);
//  											CloseHandle(pi.hProcess);
//  										}
										ShowWindow(FindWindowW(L"Shell_TrayWnd", NULL), SW_SHOW);

										std::wstring cursorFileName = windowsPath + L"aero_arrow.cur";
										ret = SetSystemCursor(LoadCursorFromFileW(cursorFileName.c_str()), OCR_NORMAL);

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
// 										HWND hWndDesk = NULL;
// 										while (hWndDesk == NULL)
// 										{
// 											hWndDesk = FindWindowW(SDL_CLASS, STEAM_DESK);
// 											if (hWndDesk)
// 											{
// 												SendMessage(hWndDesk, WM_CLOSE, 0, 0);
// 												break;
// 											}
// 											Sleep(1);
// 										}

										break;
									}
									else
									{
										HWND hWndIC = FindWindowW(ICUE_CLASS, ICUE_TITLE);
										if (hWndIC && IsWindowVisible(hWndIC))
										{
											SetWindowPos(hWndIC, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
											ShowWindow(hWndIC, SW_HIDE);
											//DestroyWindow(hWndIC);
										}

										HWND consoleHwnd = FindWindowW(L"ConsoleWindowClass", NULL);
										if (consoleHwnd)
										{
											ShowWindow(consoleHwnd, SW_HIDE);
										}
										HWND consoleHwndAlt = FindWindowW(L"CASCADIA_HOSTING_WINDOW_CLASS", NULL);
										if (consoleHwndAlt)
										{
											ShowWindow(consoleHwndAlt, SW_HIDE);
										}
										
 										if (isSteamInGame())
 										{
											isSteamFocused = false;
 										}
										}
									}
								}
								DWORD dwResult = inputHandler->GetXInputStateDeviceIO(0, &xstate); 
								if (dwResult == ERROR_SUCCESS)
								{
									if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK && !isSteamFocused)
									{
										if (!SelectButtonPressed)
										{
											if (!isSteamInGame())
											{
												HWND hWndBP2 = FindWindowW(SDL_CLASS, L"Steam Big Picture Mode");
												ShowWindow(hWndBP2, SW_MINIMIZE);
												ShowWindow(hWndBP2, SW_SHOWDEFAULT);
												SetForegroundWindow(hWndBP2);
// 												if (SUCCEEDED(hr))
// 												{
// 													BPwindow->SetFocus();
// 
// 													BPwindow->Release();
// 													BPwindow = nullptr;
// 													SelectButtonPressed = true;
// 													continue;
// 												}
											}
											SelectButtonPressed = true;
										}
									}
									else
									{
										SelectButtonPressed = false;
									}
// 									if (xstate.Gamepad.wButtons & XBOX_GUIDE && !isSteamFocused && xticksGuide.QuadPart == 0)
// 									{
// 										if (!guidePressed)
// 										{
// 											QueryPerformanceCounter(&xticksGuide);
// 											xticksGuide.QuadPart += CONTROLLER_WAKETIME;
// 											guidePressed = true;
// 										}
// 									}
// 									else if (
// 										xstate.Gamepad.wButtons & XBOX_GUIDE &&
// 										xticksGuide.QuadPart <= xticksGuide2.QuadPart &&
// 										xticksGuide2.QuadPart != 2 &&
// 										xticksGuide.QuadPart != 0)
// 									{
// 										if (!isSteamInGame())
// 										{
// 											HWND hWndBP2 = FindWindowW(SDL_CLASS, L"Steam Big Picture Mode");
// 											HRESULT hr = pAutomation->ElementFromHandle(hWndBP2, &BPwindow);
// 											ShowWindow(hWndBP2, SW_MINIMIZE);
// 											ShowWindow(hWndBP2, SW_SHOWDEFAULT);
// 											SetForegroundWindow(hWndBP2);
// 											// 												if (SUCCEEDED(hr))
// 											// 												{
// 											// 													BPwindow->SetFocus();
// 											// 
// 											// 													BPwindow->Release();
// 											// 													BPwindow = nullptr;
// 											// 													SelectButtonPressed = true;
// 											// 													continue;
// 											// 												}
// 										}
// 										xticksGuide = { 0 };
// 										xticksGuide2 = { 2 };
// 										guidePressed = true;
// 									}
// 									else
// 									{
// 										guidePressed = false;
// 									}
									if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
										xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT &&
										xticks.QuadPart == 0 && !ButtonPressed)
									{
										QueryPerformanceCounter(&xticks);
										xticks.QuadPart += MOUSE_WAKETIME / 2;
									}
									else if (
										xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
										xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT &&
										xticks.QuadPart <= xticks2.QuadPart &&
										xticks2.QuadPart != 2 &&
										xticks.QuadPart != 0)
									{
										(ShouldHideCursor) ? ShouldHideCursor = false : ShouldHideCursor = true;
										xticks = { 0 };
										xticks2 = { 2 };
										ButtonPressed = true;
									}

									if (!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ||
										!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT))
									{
										xticks = { 0 };
										xticks2 = { 2 };
										ButtonPressed = false;
									}


									if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
										xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP &&
										!TabTipCordHeld)
									{
										if (isSteamInGame())
										{
											ITipInvocation* tip = nullptr;
											if (SUCCEEDED(CoCreateInstance(CLSID_UIHostNoLaunch, 0, CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_ITipInvocation, (void**)&tip)))
											{
												tip->Toggle(GetDesktopWindow());
												tip->Release();
											}
											else if (FAILED(CoCreateInstance(CLSID_UIHostNoLaunch, 0, CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_ITipInvocation, (void**)&tip)))
											{
												WCHAR programFiles[MAX_PATH] = { 0 };
												ExpandEnvironmentStringsW(L"%PROGRAMFILES%", programFiles, MAX_PATH);
												std::wstring programFilesPath(programFiles);
												std::wstring programCommonFilesPath(programFiles);
												programCommonFilesPath = programCommonFilesPath + L"\\Common Files\\microsoft shared\\ink\\";
												programFilesPath = programFilesPath + L"\\Common Files\\microsoft shared\\ink\\TabTip.exe";
												ShellExecuteW(NULL, L"open", programFilesPath.c_str(), NULL, programCommonFilesPath.c_str(), SW_HIDE);
												Sleep(10);
												if (SUCCEEDED(CoCreateInstance(CLSID_UIHostNoLaunch, 0, CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_ITipInvocation, (void**)&tip)))
												{
													tip->Toggle(GetDesktopWindow());
													tip->Release();
												}
											}
										}
										TabTipCordHeld = true;
									}
									else if (!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ||
										!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP))
									{
										TabTipCordHeld = false;
									}

									if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
										xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
									{
										if (EnableWindowControls && !HeldEnableWindowControls)
										{
											EnableWindowControls = false;
										}
										else if (!HeldEnableWindowControls)
										{
											EnableWindowControls = true;
										}
										HeldEnableWindowControls = true;
									}
									else if (!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ||
										!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP))
									{
										HeldEnableWindowControls = false;
									}
									if (EnableWindowControls)
									{
										if (!isSteamInGame())
										{
											EnableWindowControls = false;
										}
										else
										{
											inputHandler->SendControllerInput(&xstate);
										}
									}
									QueryPerformanceCounter(&xticks2);
									QueryPerformanceCounter(&xticksGuide2);
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
		Sleep(1);
	}
	return (int)msg.wParam;
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
bool SteamHandler::getSteamFocus()
{
	return isSteamFocused;
}