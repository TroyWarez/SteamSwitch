#include "SteamHandler.h"
#include "InvisibleMouse.h"
#include <GenericInput.h>
static bool MessageBoxFound = false;
static HANDLE hShutdownEvent = NULL;
static HANDLE hSafeToRestoreEvent = NULL;
DWORD WINAPI ICUEThread(LPVOID lpParam) {
	bool iCueRunning = true;
	while (WaitForSingleObject(hShutdownEvent, 100) == WAIT_TIMEOUT)
	{
		HWND hWndIC = FindWindowW(ICUE_CLASS, ICUE_TITLE);
		if (hWndIC && IsWindowVisible(hWndIC))
		{
			ShowWindow(hWndIC, SW_HIDE);
			if (WaitForSingleObject(hSafeToRestoreEvent, 1) == WAIT_TIMEOUT && iCueRunning)
			{
				Sleep(500);
				iCueRunning = false;
				SetEvent(hSafeToRestoreEvent);
			}
		}
		Sleep(1);
	}
	HWND hWndIC = FindWindowW(ICUE_CLASS, ICUE_TITLE);
	if (hWndIC)
	{
		PostMessage(hWndIC, /*WM_QUIT*/ 0x12, 0, 0);
	}
	return 0;
}
BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId = 0;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lParam)
	{
		HWND dlgH = GetDlgItem(hwnd, (int)0xFFFF);
		if (dlgH)
		{
			if (!MessageBoxFound)
			{
				ShowWindow(dlgH, SW_MINIMIZE);
				ShowWindow(dlgH, SW_SHOWDEFAULT);
			}
			MessageBoxFound = true;
		}
		return FALSE;
	}
	return TRUE;
}

SteamHandler::SteamHandler(HWND hWnd)
{
	mainHwnd = hWnd;
	if (!hShutdownEvent)
	{
		hShutdownEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	}
	if (!hSafeToRestoreEvent)
	{
		hSafeToRestoreEvent = CreateEventW(NULL, TRUE, FALSE, L"SafeToRestoreWnd");
	}
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
	if (hShutdownEvent)
	{
		CloseHandle(hShutdownEvent);
		hShutdownEvent = NULL;
	}
	if (hSafeToRestoreEvent)
	{
		CloseHandle(hSafeToRestoreEvent);
		hSafeToRestoreEvent = NULL;
	}
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
	bool FocusCurrentWindowHeld = false;
	bool SelectButtonPressed = false;
	bool guidePressed = false;
	bool TopMost = false;
	bool ShouldHideCursor = true;
	bool ButtonPressed = false;
	bool AButtonPressed = false;
	bool StartButtonPressed = false;
	bool ShouldRightClick = true;
	bool iCueRunning = false;
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
	POINT deskCursorPos = { 0 };

	MSG msg;
	WCHAR programFiles[MAX_PATH] = { 0 };
	ExpandEnvironmentStringsW(L"%PROGRAMFILES%", programFiles, MAX_PATH);
	std::wstring programFilesPath(programFiles);
	programFilesPath = programFilesPath + L"\\Corsair\\Corsair iCUE5 Software\\iCUE Launcher.exe";
	HANDLE hiCueTestFile = CreateFileW(programFilesPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hiCueTestFile == INVALID_HANDLE_VALUE)
	{
		programFilesPath = L"";
	}
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
						GetCursorPos(&deskCursorPos);
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
						LONG wndLong = GetWindowLongW(foreHwnd, GWL_STYLE);
						SetWindowPos(foreHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
						if (!monHandler->ToggleMode())
						{
							INPUT inputs[4] = {};
							ZeroMemory(inputs, sizeof(inputs));

							inputs[0].type = INPUT_KEYBOARD;
							inputs[0].ki.wVk = VK_MENU;

							inputs[1].type = INPUT_KEYBOARD;
							inputs[1].ki.wVk = VK_RETURN;

							inputs[2].type = INPUT_KEYBOARD;
							inputs[2].ki.wVk = VK_MENU;
							inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

							inputs[3].type = INPUT_KEYBOARD;
							inputs[3].ki.wVk = VK_RETURN;
							inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

							UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
							Sleep(500);
							continue;
						}

						SetCursorPos(((GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1) * 2), ((GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1) * 2));
						if (programFilesPath != L"")
						{
							if (hShutdownEvent)
							{
								ResetEvent(hShutdownEvent);
							}
							if (hSafeToRestoreEvent)
							{
								ResetEvent(hSafeToRestoreEvent);
							}
							CreateThread(NULL, 0, ICUEThread, NULL, 0, NULL);
							HWND bpHwnd = FindWindowW(SDL_CLASS, title.c_str());
							SHELLEXECUTEINFOW sei = { sizeof(SHELLEXECUTEINFO) };
							sei.fMask = SEE_MASK_NOCLOSEPROCESS; // Request process handle
							sei.lpFile = programFilesPath.c_str();        // File to execute
							sei.nShow = SW_MINIMIZE;       // How to show the window
							HANDLE hProcessiCue = NULL;

							if (ShellExecuteExW(&sei)) {
								if (sei.hProcess != NULL) {
									hProcessiCue = sei.hProcess;
									iCueRunning = true;
								}
							}
						}
						steamBigPictureModeTitle = title;

						//HWND eH = FindWindowW(L"Progman", NULL);
						//GetWindowThreadProcessId(eH, &PID);
						//PostMessage(eH, /*WM_QUIT*/ 0x12, 0, 0);
						isSteamInBigPictureMode = true;
						ShowWindow(FindWindowW(L"Shell_TrayWnd", NULL), SW_HIDE);

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
										if (programFiles != L"")
										{
											SetEvent(hShutdownEvent);
										}
										inputHandler->turnOffXinputController();

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
										SetCursorPos(deskCursorPos.x, deskCursorPos.y);
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
										if (iCueRunning && hWndBP && (WaitForSingleObject(hSafeToRestoreEvent, 1) != WAIT_TIMEOUT))
										{
											SetWindowPos(hWndBP, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
											SetWindowLongW(hWndBP, GWL_STYLE, wndLong);
											iCueRunning = false;
										}
										if (!isSteamInGame())
										{

											HWND consoleHwnd = FindWindowW(L"ConsoleWindowClass", NULL);
											if (consoleHwnd)
											{
												ShowWindow(consoleHwnd, SW_MINIMIZE);
											}
											HWND consoleHwndAlt = FindWindowW(L"CASCADIA_HOSTING_WINDOW_CLASS", NULL);
											if (consoleHwndAlt)
											{
												ShowWindow(consoleHwndAlt, SW_MINIMIZE);
											}
										}
										}
									}
								}
								DWORD dwResult = inputHandler->GetXInputStateDeviceIO(0, &xstate); 
								if (dwResult == ERROR_SUCCESS)
								{
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
										!SelectButtonPressed)
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
									else if (!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK))
									{
										SelectButtonPressed = false;
									}


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

									if (MessageBoxFound)
									{
										if (isSteamInGame())
										{
											inputHandler->SendControllerInput(&xstate);
										}
									}
									QueryPerformanceCounter(&xticks2);
									QueryPerformanceCounter(&xticksGuide2);
								}
								else {
									for (DWORD i = 0; i < 12; i++)
									{
										xstate = { 0 };
										dwResult = GenericInputGetState(i, (GENERIC_INPUT_STATE*)&xstate);
										if (dwResult == ERROR_SUCCESS) {
											if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
												!SelectButtonPressed)
											{

												if (!isSteamInGame())
												{
													HWND hWndBP2 = FindWindowW(SDL_CLASS, L"Steam Big Picture Mode");
													ShowWindow(hWndBP2, SW_MINIMIZE);
													ShowWindow(hWndBP2, SW_SHOWDEFAULT);
													SetForegroundWindow(hWndBP2);
												}
												SelectButtonPressed = true;
											}
											else if (!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK))
											{
												SelectButtonPressed = false;
											}


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
										}
										else
										{
											break;
										}
									}
									if (MessageBoxFound)
									{
										if (isSteamInGame())
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

							if (pbi[5] == steamPid && processName != L"steamwebhelper.exe" )
								{
								if (processName != L"gameoverlayui64.exe" && processName != L"gameoverlayui.exe")
								{
									gamePid = processIds[i];
									EnumWindows(EnumWindowsProcMy, gamePid);
								}

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