#include "SteamHandler.h"
#include "InvisibleMouse.h"
#include <GenericInput.h>
BOOL                AddOrRemoveNotificationIcon(HWND, BOOL);
static bool MessageBoxFound = false;
DWORD WINAPI ICUEThread(LPVOID lpParam) {
	UNREFERENCED_PARAMETER(lpParam);
	HANDLE hEvents[3] = { nullptr };
	hEvents[0] = OpenEventW(SYNCHRONIZE, FALSE, L"ShutdownEvent");
	if (hEvents[0] == nullptr)
	{
		return 1;
	}
	hEvents[1] = CreateEventW(nullptr, TRUE, FALSE, L"FindIcueEvent");
	if (!hEvents[1] && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hEvents[1] = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"FindIcueEvent");
		if (hEvents[1])
		{
			ResetEvent(hEvents[1]);
		}
	}
	if (hEvents[1] == nullptr)
	{
		return 1;
	}

	hEvents[2] = CreateEventW(nullptr, TRUE, FALSE, L"CloseIcueEvent");
	if (!hEvents[2] && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hEvents[2] = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"CloseIcueEvent");
		if (hEvents[2])
		{
			ResetEvent(hEvents[2]);
		}
	}
	if (hEvents[2] == nullptr)
	{
		return 1;
	}

	bool iCueRunning = true;
	HANDLE hICUEEvent = CreateEventW(nullptr, TRUE, FALSE, L"ICUEEvent");
	if (!hICUEEvent && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hICUEEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"ICUEEvent");
		if (hICUEEvent)
		{
			ResetEvent(hICUEEvent);
		}
	}
	while (iCueRunning)
	{
		DWORD dwWait = WaitForMultipleObjects(ARRAYSIZE(hEvents), hEvents, FALSE, INFINITE);
		switch (dwWait)
		{
			case 0: // Shutdown Event
			{
				iCueRunning = false;
				break;
			}
			case 1: // FindIcueEvent Event
			{
				HWND hWndIC = FindWindowW(ICUE_CLASS, ICUE_TITLE);
				if (hWndIC && IsWindowVisible(hWndIC))
				{
					ShowWindow(hWndIC, SW_HIDE);
					if (hICUEEvent && WaitForSingleObject(hICUEEvent, 1) == WAIT_TIMEOUT)
					{
						SetEvent(hICUEEvent);
						ResetEvent(hEvents[1]);
					}
				}
				Sleep(1);
				break;
			}
			case 2: // CloseIcueEvent Event
			{
				HWND hWndIC = FindWindowW(ICUE_CLASS, ICUE_TITLE);
				if (hWndIC)
				{
					SetForegroundWindow(hWndIC);
					PostMessage(hWndIC, WM_ENDSESSION, 0, 0);
					ResetEvent(hEvents[2]);
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}
	for (int i = 0; i < ARRAYSIZE(hEvents); i++)
	{
		if (hEvents[i])
		{
			CloseHandle(hEvents[i]);
			hEvents[i] = nullptr;
		}
	}
	return 0;
}
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
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

	DEV_BROADCAST_DEVICEINTERFACE hidFilter = { };
	hidFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	hidFilter.dbcc_classguid = GUID_DEVINTERFACE_COMPORT;
	hidFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	HDEVNOTIFY hDeviceHID = RegisterDeviceNotificationW(mainHwnd, &hidFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

	hBPEvent = CreateEventW(nullptr, FALSE, FALSE, L"BPEvent");
	if (hBPEvent == nullptr && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hBPEvent = OpenEventW(SYNCHRONIZE, FALSE, L"BPEvent");
		if (hBPEvent)
		{
			ResetEvent(hBPEvent);
		}
	}
	hShutdownEvent = CreateEventW(nullptr, TRUE, FALSE, L"ShutdownEvent");
	if (hShutdownEvent == nullptr && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hShutdownEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"ShutdownEvent");
		if (hShutdownEvent)
		{
			ResetEvent(hShutdownEvent);
		}
	}

	hFindIcueEvent = CreateEventW(nullptr, TRUE, FALSE, L"FindIcueEvent");
	if (hFindIcueEvent == nullptr && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hFindIcueEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"FindIcueEvent");
		if (hFindIcueEvent)
		{
			ResetEvent(hFindIcueEvent);
		}
	}

	hCloseIcueEvent = CreateEventW(nullptr, TRUE, FALSE, L"CloseIcueEvent");
	if (hCloseIcueEvent == nullptr && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hCloseIcueEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"CloseIcueEvent");
		if (hCloseIcueEvent)
		{
			ResetEvent(hCloseIcueEvent);
		}
	}

	hICUEThread = nullptr;
	hCECThread = nullptr;
	hSerialThread = nullptr;

	steamPid = getSteamPid();
	gamePid = 0;
	isIcueInstalled = false;
	isSteamFocused = true;
	monHandler = new MonitorHandler(MonitorHandler::DESK_MODE);
	inputHandler = new InputHandler();
	audioHandler = new AudioHandler();
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
		hShutdownEvent = nullptr;
	}
	if (hBPEvent)
	{
		CloseHandle(hBPEvent);
		hBPEvent = nullptr;
	}
	if (hFindIcueEvent)
	{
		CloseHandle(hFindIcueEvent);
		hFindIcueEvent = nullptr;
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
	if (audioHandler)
	{
		delete audioHandler;
	}
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

	bool MouseCordPressed = false;
	bool AudioCordPressed = false;

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
	LONG wndLong = 0;

	LARGE_INTEGER ticks = { 0 };
	LARGE_INTEGER ticks2 = { 2 };

	LARGE_INTEGER xticks = { 0 };
	LARGE_INTEGER xticks2 = { 2 };

	LARGE_INTEGER xticksAudio = { 0 };
	LARGE_INTEGER xticksAudio2 = { 2 };

	XINPUT_STATE xstate = { 0 };

	POINT firstCursorPos = { 0 };
	POINT deskCursorPos = { 0 };

	MSG msg;
	std::array<WCHAR, MAX_PATH> programFiles = { 0 };
	ExpandEnvironmentStringsW(L"%PROGRAMFILES%\\Corsair\\Corsair iCUE5 Software\\iCUE Launcher.exe", programFiles.data(), MAX_PATH);
	std::wstring programFilesPath(programFiles.data());
	programFilesPath = programFilesPath;
	HANDLE hiCueTestFile = CreateFileW(programFilesPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hiCueTestFile == INVALID_HANDLE_VALUE)
	{
		programFilesPath = L"";
	}
	else
	{
		isIcueInstalled = true;
		CloseHandle(hiCueTestFile);
		if (hICUEThread == nullptr)
		{
			hICUEThread = CreateThread(nullptr, 0, ICUEThread, nullptr, 0, nullptr);
		}
	}
	serialHandler.ScanForSerialDevices();
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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
				if (hWnd == nullptr)// Don't place anything here.
				{
					HWND foreHwnd = GetForegroundWindow();

					WCHAR windowTitle[256] = { 0 };
					GetWindowTextW(foreHwnd, windowTitle, 256);
					std::wstring title(windowTitle);
					std::wstring subtitle;
					if (title.size() > 5)
					{
						subtitle = title.substr(0, 5);
					}
					WCHAR windowClassName[256] = { 0 };
					GetClassNameW(foreHwnd, windowClassName, 256);
					std::wstring classname(windowClassName);

					if (subtitle == STEAM_DESK && classname == SDL_CLASS && title != subtitle)
					{
						// Entering Big Picture Mode
						AddOrRemoveNotificationIcon(hWnd, TRUE);
						if (!monHandler->isSingleDisplayHDMI())
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
						}

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

						if (!monHandler->ToggleMode((!programFilesPath.empty())))
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
							continue;
						}

						SetCursorPos(((GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1) * 2), ((GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1) * 2));
						if (!programFilesPath.empty())
						{
							HWND bpHwnd = FindWindowW(SDL_CLASS, title.c_str());
							SHELLEXECUTEINFOW sei = { sizeof(SHELLEXECUTEINFO) };
							sei.fMask = SEE_MASK_NOCLOSEPROCESS; // Request process handle
							sei.lpFile = programFilesPath.c_str();        // File to execute
							sei.nShow = SW_MINIMIZE;       // How to show the window
							HANDLE hProcessiCue = nullptr;

							if (ShellExecuteExW(&sei)) {
								if (sei.hProcess != nullptr) {
									hProcessiCue = sei.hProcess;
									iCueRunning = true;
								}
							}
							if (hFindIcueEvent && WaitForSingleObject(hFindIcueEvent, 1) == WAIT_TIMEOUT)
							{
								SetEvent(hFindIcueEvent);
							}
						}
						steamBigPictureModeTitle = title;

						//HWND eH = FindWindowW(L"Progman", nullptr);
						//GetWindowThreadProcessId(eH, &PID);
						//PostMessage(eH, /*WM_QUIT*/ 0x12, 0, 0);
						isSteamInBigPictureMode = true;
						ShowWindow(FindWindowW(L"Shell_TrayWnd", nullptr), SW_HIDE);
						while (true)
						{
							if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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
								if (hBPEvent && WaitForSingleObject(hBPEvent, 1) != WAIT_TIMEOUT)
								{
									ResetEvent(hBPEvent);
									break;
								}
							}
						}
						while (isSteamInBigPictureMode)
						{
							if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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
									if (hWndBP == nullptr) { // Big picture mode closed
										AddOrRemoveNotificationIcon(hWnd, TRUE);
										if (!programFilesPath.empty())
										{
											if (hCloseIcueEvent && WaitForSingleObject(hCloseIcueEvent, 1) == WAIT_TIMEOUT)
											{
												SetEvent(hCloseIcueEvent);
											}
										}
										inputHandler->turnOffXinputController();

										HWND hWndDesk = FindWindowW(SDL_CLASS, STEAM_DESK);
										//SetWindowLongW(hWndDesk, GWL_STYLE, wndLong);
										monHandler->ToggleMode((!programFilesPath.empty()));
										ShowWindow(FindWindowW(L"Shell_TrayWnd", nullptr), SW_SHOW);
										SetCursorPos(deskCursorPos.x, deskCursorPos.y);

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

										isSteamInBigPictureMode = false;

										break;
									}
									if (iCueRunning && hWndBP &&
										!programFilesPath.empty())
									{
										iCueRunning = false;
									}
									if (!isSteamInGame())
									{

										HWND consoleHwnd = FindWindowW(L"ConsoleWindowClass", nullptr);
										if (consoleHwnd)
										{
											ShowWindow(consoleHwnd, SW_MINIMIZE);
										}
										HWND consoleHwndAlt = FindWindowW(L"CASCADIA_HOSTING_WINDOW_CLASS", nullptr);
										if (consoleHwndAlt)
										{
											ShowWindow(consoleHwndAlt, SW_MINIMIZE);
										}
									}
								}
								}
								DWORD dwResult = inputHandler->GetXInputStateDeviceIO(0, &xstate); 
								if (dwResult == ERROR_SUCCESS)
								{
									if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
										!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) &&
										!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B) &&
										!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) &&
										!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) &&
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
										xticks.QuadPart == 0 && !MouseCordPressed)
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
										MouseCordPressed = true;
									}

									if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
										xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B &&
										xticksAudio.QuadPart == 0 && !AudioCordPressed)
									{
										QueryPerformanceCounter(&xticksAudio);
										xticksAudio.QuadPart += MOUSE_WAKETIME / 2;
									}
									else if (
										xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
										xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B &&
										xticksAudio.QuadPart <= xticksAudio2.QuadPart &&
										xticksAudio2.QuadPart != 2 &&
										xticksAudio.QuadPart != 0)
									{
										audioHandler->ToggleAudioDevice();

										xticksAudio = { 0 };
										xticksAudio2 = { 2 };
										AudioCordPressed = true;
									}

									if (!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ||
										!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B))
									{
										xticksAudio = { 0 };
										xticksAudio2 = { 2 };
										AudioCordPressed = false;
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
												std::array<WCHAR, MAX_PATH> programFiles = { 0 };
												ExpandEnvironmentStringsW(L"%PROGRAMFILES%", programFiles.data(), MAX_PATH);
												std::wstring programFilesPath(programFiles.data());
												std::wstring programCommonFilesPath(programFiles.data());
												programCommonFilesPath = programCommonFilesPath + L"\\Common Files\\microsoft shared\\ink\\";
												programFilesPath = programFilesPath + L"\\Common Files\\microsoft shared\\ink\\TabTip.exe";
												ShellExecuteW(nullptr, L"open", programFilesPath.c_str(), nullptr, programCommonFilesPath.c_str(), SW_HIDE);
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
									QueryPerformanceCounter(&xticksAudio2);
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
													HWND hWndBP2 = FindWindowW(SDL_CLASS, title.c_str());
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
												xticks.QuadPart == 0 && !MouseCordPressed)
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
												MouseCordPressed = true;
											}

											if (!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ||
												!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT))
											{
												xticks = { 0 };
												xticks2 = { 2 };
												MouseCordPressed = false;
											}

											if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
												xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B &&
												xticksAudio.QuadPart == 0 && !AudioCordPressed)
											{
												QueryPerformanceCounter(&xticksAudio);
												xticksAudio.QuadPart += MOUSE_WAKETIME / 2;
											}
											else if (
												xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK &&
												xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B &&
												xticksAudio.QuadPart <= xticksAudio2.QuadPart &&
												xticksAudio2.QuadPart != 2 &&
												xticksAudio.QuadPart != 0)
											{
												audioHandler->ToggleAudioDevice();
												PlaySoundW(L"SystemExclamation", nullptr, SND_ALIAS | SND_ASYNC);
												xticksAudio = { 0 };
												xticksAudio2 = { 2 };
												AudioCordPressed = true;
											}

											if (!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ||
												!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B))
											{
												xticksAudio = { 0 };
												xticksAudio2 = { 2 };
												AudioCordPressed = false;
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
									QueryPerformanceCounter(&xticksAudio2);
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
	HKEY hKey = nullptr;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD type = REG_DWORD;
		DWORD pid = 0;
		DWORD pidSize = sizeof(pid);
		if (RegQueryValueExW(hKey, L"pid", nullptr, &type, (BYTE*)&pid, &pidSize) == ERROR_SUCCESS)
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
	ULONG_PTR pbi[6] = { 0 };
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
									EnumWindows(EnumWindowsProc, gamePid);
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
const bool SteamHandler::getSteamFocus()
{
	return isSteamFocused;
}