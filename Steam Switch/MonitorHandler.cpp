#include "MonitorHandler.h"
#include "SteamHandler.h"
#include "AudioHandler.h"
#include <iostream>
#include <cecloader.h>
#include <vector>
#include <debugapi.h>
extern AudioHandler audioHandler;
// It may be be possible to have two cec usb devices on the same cable
DWORD WINAPI CecPowerThread(LPVOID lpParam) {
	bool openedBPMode = false;
	SteamHandler* steamHandler = (SteamHandler*)lpParam;
	CEC::libcec_configuration cec_config;
	std::string deviceStrPort = "";
	CEC::ICECAdapter* cecAdpater;
	cec_config.Clear();
	cec_config.iHDMIPort = 1; // Default HDMI port

	cec_config.clientVersion = CEC::LIBCEC_VERSION_CURRENT;
	cec_config.bActivateSource = 0;
	cec_config.bAutoPowerOn = 0;
	cec_config.iPhysicalAddress = 0;
	cec_config.bAutodetectAddress = 0;
	cec_config.bGetSettingsFromROM = 0;
	cec_config.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);

	//bool ret = cecAdpater->SetConfiguration(&cec_config);
	WCHAR programFiles[MAX_PATH] = { 0 };
	ExpandEnvironmentStringsW(L"%userProfile%", programFiles, MAX_PATH);
	std::wstring programFilesPath(programFiles);
	programFilesPath = programFilesPath + L"\\SteamSwitch\\cecHDMI_Port.txt";
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFileW(programFilesPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD bytesRead;
		CHAR buffer[MAX_PATH] = { 0 };
		WCHAR wbuffer[MAX_PATH] = { 0 };
		bytesRead = GetFileSize(hFile, NULL);
		if (bytesRead > 0 && bytesRead < MAX_PATH)
		{
			if (ReadFile(hFile, buffer, bytesRead, &bytesRead, NULL))
			{
				cec_config.iHDMIPort = std::stoi(buffer);
			}
		}
		CloseHandle(hFile);
	}
	cecAdpater = LibCecInitialise(&cec_config);

	if (cecAdpater == NULL)
	{
		return 1;
	}
	cecAdpater->InitVideoStandalone();
	CEC::cec_adapter_descriptor device[1];
	uint8_t iDevicesFound = cecAdpater->DetectAdapters(device, 1, NULL, true);
	if (iDevicesFound > 0)
	{
		deviceStrPort = device[0].strComName;
	}
	HANDLE hICUEEvent = CreateEventW(NULL, FALSE, FALSE, L"ICUEEvent");
	if (hICUEEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hICUEEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ICUEEvent");
		if (hICUEEvent)
		{
			ResetEvent(hICUEEvent);
		}
	}

	HANDLE hCECPowerOffFinishedEvent = CreateEventW(NULL, FALSE, FALSE, L"CECPowerOffFinishedEvent");
	if (hCECPowerOffFinishedEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hCECPowerOffFinishedEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"CECPowerOffFinishedEvent");
		if (hCECPowerOffFinishedEvent)
		{
			ResetEvent(hCECPowerOffFinishedEvent);
		}
	}

	HANDLE hShutdownEvent = CreateEventW(NULL, FALSE, FALSE, L"ShutdownEvent");
	if (hShutdownEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hShutdownEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ShutdownEvent");
		if (hShutdownEvent)
		{
			ResetEvent(hShutdownEvent);
		}
	}
	HANDLE hCECPowerOffEvent = CreateEventW(NULL, FALSE, FALSE, L"CECPowerOffEvent");
	if (hCECPowerOffEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hCECPowerOffEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"CECPowerOffEvent");
		if (hCECPowerOffEvent)
		{
			ResetEvent(hCECPowerOffEvent);
		}
	}

	HANDLE hCECPowerOnEventSerial = CreateEventW(NULL, FALSE, FALSE, L"CECPowerOnEventSerial");
	if (hCECPowerOnEventSerial == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hCECPowerOnEventSerial = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"CECPowerOnEventSerial");
		if (hCECPowerOnEventSerial)
		{
			ResetEvent(hCECPowerOnEventSerial);
		}
	}

	HANDLE hCECPowerOnEvent = CreateEventW(NULL, FALSE, FALSE, L"CECPowerOnEvent");
	if (hCECPowerOnEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hCECPowerOnEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"CECPowerOnEvent");
		if (hCECPowerOnEvent)
		{
			ResetEvent(hCECPowerOnEvent);
		}
	}

	HANDLE hBPEvent = CreateEventW(NULL, FALSE, FALSE, L"BPEvent");
	if (hBPEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hBPEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"BPEvent");
		if (hBPEvent)
		{
			ResetEvent(hBPEvent);
		}
	}
	std::vector<HANDLE> hEvents;

	if (!hShutdownEvent)
	{
		return 1;
	}
	hEvents.push_back(hShutdownEvent);
	hEvents.push_back(hCECPowerOnEvent);
	hEvents.push_back(hCECPowerOffEvent);
	hEvents.push_back(hCECPowerOnEventSerial);

	BOOL SingleDisplayHDMI = FALSE;
	if (steamHandler && steamHandler->monHandler && steamHandler->monHandler->isSingleDisplayHDMI())
	{
		SetEvent(hCECPowerOnEvent);
		SingleDisplayHDMI = TRUE;
	}

	if (cecAdpater && deviceStrPort != "")
	{
		DWORD dwWaitResult = 0;
		while (dwWaitResult <= ((DWORD)hEvents.size() - 1)){
		dwWaitResult = WaitForMultipleObjects((DWORD)hEvents.size(), hEvents.data(), FALSE, INFINITE);
		switch (dwWaitResult)
		{
		case 0: // hShutdownEvent
			{
				for (size_t i = 0; i < hEvents.size(); i++)
				{
					if (hEvents[i] != NULL)
					{
						CloseHandle(hEvents[i]);
					}
				}
				if (cecAdpater)
				{
					UnloadLibCec(cecAdpater);
					cecAdpater = 0;
				}
				return 0;
			}
		case 3: // hCECPowerOnEventSerial
		{
			openedBPMode = false;
			if (hCECPowerOnEventSerial)
			{
				ResetEvent(hCECPowerOnEventSerial);
				if (hCECPowerOnEvent)
				{
					SetEvent(hCECPowerOnEvent);
				}
			}
			break;
		}
		case 1: // hCECPowerOnEvent
		{
			if (hCECPowerOnEvent)
			{
				ResetEvent(hCECPowerOnEvent);
			}
			cecAdpater->Open(deviceStrPort.c_str());
			if (cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_ON)
			{
				cecAdpater->SetActiveSource(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);
			}
			else if (cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_ON ||
				cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON)
			{
				cecAdpater->SetActiveSource(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);
				if (cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_ON ||
					cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON)
				{
					cecAdpater->SetActiveSource(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);
				}
			}
		while ((cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_ON) && (cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_UNKNOWN) && WaitForSingleObject(hShutdownEvent, 1) == WAIT_TIMEOUT)
			{
				cecAdpater->SetActiveSource(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);
				Sleep(1);
			}
			cecAdpater->Close();
			if (!SingleDisplayHDMI)
			{
				if (FindWindowW(SDL_CLASS, STEAM_DESK))
				{
					ShellExecuteW(GetDesktopWindow(), L"open", L"steam://open/bigpicture", NULL, NULL, SW_SHOW);
				}
				else
				{
					ShellExecuteW(GetDesktopWindow(), L"open", L"steam://open/", NULL, NULL, SW_SHOW);
				}
			}
			while (WaitForSingleObject(hShutdownEvent, 1) == WAIT_TIMEOUT && !audioHandler.BPisDefaultAudioDevice())
			{
				audioHandler.InitDefaultAudioDevice();
				Sleep(100);
			}
			while (hShutdownEvent && WaitForSingleObject(hShutdownEvent, 1) == WAIT_TIMEOUT)
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
					if (hBPEvent)
					{
						SetEvent(hBPEvent);
					}
					if (hICUEEvent)
					{
						SetWindowPos(foreHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
						WaitForSingleObject(hICUEEvent, 60000);
						Sleep(400);
						SetWindowPos(foreHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
						CloseHandle(hICUEEvent);
						hICUEEvent = NULL;
					}
					break;
				}
				else if (SingleDisplayHDMI)
				{
					HWND hWnd = FindWindowW(SDL_CLASS, STEAM_DESK);
					if (hWnd == NULL)
					{
						ShellExecuteW(GetDesktopWindow(), L"open", L"steam://open/", NULL, NULL, SW_SHOW);
						Sleep(3000);
					}
					else if (!openedBPMode)
					{
						ShowWindow(hWnd, SW_MINIMIZE);
						ShowWindow(hWnd, SW_SHOWDEFAULT);
						SetForegroundWindow(hWnd);
						ShellExecuteW(GetDesktopWindow(), L"open", L"steam://open/bigpicture", NULL, NULL, SW_SHOW);
						openedBPMode = true;
					}
					continue;
				}
				Sleep(1);
			}
			break;
		}
		case 2: // hCECPowerOffEvent
		{
			if (hCECPowerOffEvent)
			{
				ResetEvent(hCECPowerOffEvent);
			}
				cecAdpater->Open(deviceStrPort.c_str());
				if (cecAdpater->GetActiveSource() != CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE)
				{
					cecAdpater->SetActiveSource(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);
				}
				cecAdpater->StandbyDevices(CEC::CECDEVICE_TV);

				if (cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_STANDBY ||
					cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY)
				{
					cecAdpater->StandbyDevices(CEC::CECDEVICE_TV);
					if (cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_STANDBY ||
						cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY)
					{
						cecAdpater->StandbyDevices(CEC::CECDEVICE_TV);
					}
				}
				cecAdpater->Close();
				if (hCECPowerOffFinishedEvent)
				{
					SetEvent(hCECPowerOffFinishedEvent);
				}
			}
			break;
		}
		}
		}
	if (cecAdpater)
	{
		UnloadLibCec(cecAdpater);
		cecAdpater = 0;
	}
	return 0;
}
void MonitorHandler::StartCecPowerThread(void* stmPtr)
{
	if (hCECThread == NULL || stmPtr == nullptr)
	{
		hCECThread = CreateThread(NULL, 0, CecPowerThread, stmPtr, 0, NULL);
	}
}
MonitorHandler::MonitorHandler(MonitorMode mode)
{
	currentMode = mode;
	hCECThread = NULL;
	hCECPowerOffEvent = CreateEventW(NULL, FALSE, FALSE, L"CECPowerOffEvent");
	hCECPowerOffFinishedEvent = CreateEventW(NULL, FALSE, FALSE, L"CECPowerOffFinishedEvent");
	hCECPowerOnEvent = CreateEventW(NULL, FALSE, FALSE, L"CECPowerOnEvent");
	hShutdownEvent = CreateEventW(NULL, FALSE, FALSE, L"ShutdownEvent");
	icueInstalled = false;
}
MonitorHandler::~MonitorHandler()
{
	if (hShutdownEvent)
	{
		SetEvent(hShutdownEvent);
		CloseHandle(hShutdownEvent);
		hShutdownEvent = NULL;
	}
}
void MonitorHandler::setMonitorMode(MonitorMode mode)
{
	currentMode = mode;
}
MonitorHandler::MonitorMode MonitorHandler::getMonitorMode()
{
	return currentMode;
}
bool MonitorHandler::ToggleMode(bool isIcueInstalled)
{
	switch (currentMode)
	{
		case MonitorHandler::BP_MODE:
		{
			ToggleActiveMonitors(MonitorHandler::DESK_MODE);
			currentMode = MonitorHandler::DESK_MODE;
			if (hCECPowerOffEvent && hCECPowerOnEvent)
			{
				SetEvent(hCECPowerOffEvent);
				ResetEvent(hCECPowerOnEvent);
			}
			break;
		}
		case MonitorHandler::DESK_MODE:
		{
			if (!ToggleActiveMonitors(MonitorHandler::BP_MODE))
			{
				return false;
			}
			if (hCECPowerOffEvent && hCECPowerOnEvent)
			{
				SetEvent(hCECPowerOnEvent);
				ResetEvent(hCECPowerOffEvent);
			}
			currentMode = MonitorHandler::BP_MODE;
			break;
		}
	}
	return true;
}
bool MonitorHandler::ToggleActiveMonitors(MonitorMode mode)
{
	if (mode == DESK_MODE)
	{
		SetDisplayConfig(0, NULL, 0, NULL, SDC_TOPOLOGY_EXTEND | SDC_APPLY);
	}

	if (mode == BP_MODE)
	{
		if (isDSCEnabled())
		{
			WCHAR windowsDir[MAX_PATH] = { 0 };
			std::wstring windowsPath(windowsDir);
			if (GetWindowsDirectoryW(windowsDir, MAX_PATH))
			{
				windowsPath = windowsDir;
				windowsPath = windowsPath + L"\\Cursors\\";
			}
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

			if (icueInstalled)
			{
				SetWindowPos(GetForegroundWindow(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}

			while (isDSCEnabled())
			{
				int msgboxID = MessageBoxW(
					NULL,
					(LPCWSTR)L"Display stream compression (DSC) is turned on and must be turned off to use Big Picture Mode.\nDo you want to try again?",
					(LPCWSTR)L"Steam Switch Critical Display Error",
					MB_ICONERROR | MB_RETRYCANCEL | MB_DEFBUTTON2 | MB_TOPMOST
				);

				switch (msgboxID)
				{
				case IDCANCEL:
					return false;
				}
			}
			if (!isDSCEnabled())
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
			}
		}

	}
	HRESULT hr = S_OK;
	UINT32 NumPathArrayElements = 0;
	UINT32 NumModeInfoArrayElements = 0;
	hr = GetDisplayConfigBufferSizes((QDC_ALL_PATHS), &NumPathArrayElements, &NumModeInfoArrayElements);
	std::vector<DISPLAYCONFIG_PATH_INFO> PathInfoArray2(NumPathArrayElements);
	DISPLAYCONFIG_PATH_INFO pathInfoHDMI = {};
	pathInfoHDMI.targetInfo.outputTechnology = DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER;
	std::vector<DISPLAYCONFIG_MODE_INFO> ModeInfoArray2(NumModeInfoArrayElements);
	hr = QueryDisplayConfig((QDC_ALL_PATHS), &NumPathArrayElements, &PathInfoArray2[0], &NumModeInfoArrayElements, &ModeInfoArray2[0], NULL);

	int HDMIMonitorCount = 0;
	int DPMonitorCount = 0;

	if (hr == S_OK)
	{

		DISPLAYCONFIG_SOURCE_DEVICE_NAME SourceName = {};
		SourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
		SourceName.header.size = sizeof(SourceName);

		DISPLAYCONFIG_TARGET_PREFERRED_MODE PreferedMode = {};
		PreferedMode.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_PREFERRED_MODE;
		PreferedMode.header.size = sizeof(PreferedMode);


		int newId = 0;

		if (hr == S_OK)
		{
			for (UINT32 i = 0; i < NumPathArrayElements; i++)
			{
				SourceName.header.adapterId = PathInfoArray2[i].sourceInfo.adapterId;
				SourceName.header.id = PathInfoArray2[i].sourceInfo.id;

				PreferedMode.header.adapterId = PathInfoArray2[i].targetInfo.adapterId;
				PreferedMode.header.id = PathInfoArray2[i].targetInfo.id;

				hr = HRESULT_FROM_WIN32(DisplayConfigGetDeviceInfo(&SourceName.header));
				hr = HRESULT_FROM_WIN32(DisplayConfigGetDeviceInfo(&PreferedMode.header));

				if (hr == S_OK)
				{

					if (PathInfoArray2[i].targetInfo.targetAvailable == 1)
					{
						PathInfoArray2[i].sourceInfo.id = newId;
						newId++;
					}

					if (PathInfoArray2[i].targetInfo.id != PreferedMode.header.id)
					{
						PathInfoArray2[i].targetInfo.id = PreferedMode.header.id;
					}

					PathInfoArray2[i].sourceInfo.modeInfoIdx = DISPLAYCONFIG_PATH_MODE_IDX_INVALID;
					PathInfoArray2[i].targetInfo.modeInfoIdx = DISPLAYCONFIG_PATH_MODE_IDX_INVALID;
				}
				if (PathInfoArray2[i].targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI)
				{
					if (pathInfoHDMI.targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER)
					{
						pathInfoHDMI = PathInfoArray2[i];
						pathInfoHDMI.flags = DISPLAYCONFIG_PATH_ACTIVE;
					}
					PathInfoArray2[i].flags = 0;
				}
			}
			if (mode == BP_MODE)
			{
				DISPLAYCONFIG_PATH_INFO pathInfo[2] = {};
				pathInfo[0] = PathInfoArray2[1];
				pathInfo[1] = pathInfoHDMI;

				PathInfoArray2[0] = pathInfoHDMI;
				PathInfoArray2[2] = pathInfo[0];
				PathInfoArray2[1] = pathInfo[1];

				PathInfoArray2[0].flags = DISPLAYCONFIG_PATH_ACTIVE;
				PathInfoArray2[1].flags = 0;
				PathInfoArray2[2].flags = 0;
				hr = SetDisplayConfig(NumPathArrayElements, &PathInfoArray2[0], 0, NULL, (SDC_VALIDATE | SDC_TOPOLOGY_SUPPLIED | SDC_ALLOW_PATH_ORDER_CHANGES));
				if (hr == S_OK)
				{
					hr = SetDisplayConfig(NumPathArrayElements, &PathInfoArray2[0], 0, NULL, (SDC_APPLY | SDC_TOPOLOGY_SUPPLIED | SDC_ALLOW_PATH_ORDER_CHANGES));
				}
			}
			else if (mode == DESK_MODE)
			{
				DISPLAYCONFIG_PATH_INFO pathInfo[2] = {};
				pathInfo[0] = PathInfoArray2[0];
				pathInfo[1] = PathInfoArray2[1];

				PathInfoArray2[0] = pathInfo[1];
				PathInfoArray2[1] = pathInfoHDMI;
				PathInfoArray2[2] = PathInfoArray2[0];

				PathInfoArray2[0].flags = DISPLAYCONFIG_PATH_ACTIVE;
				PathInfoArray2[1].flags = DISPLAYCONFIG_PATH_ACTIVE;

				constexpr UINT flags = SDC_APPLY |
					SDC_SAVE_TO_DATABASE |
					SDC_ALLOW_CHANGES |
					SDC_USE_SUPPLIED_DISPLAY_CONFIG |
					SDC_TOPOLOGY_EXTEND;
				hr = SetDisplayConfig(NumPathArrayElements, &PathInfoArray2[0], 0, NULL, (SDC_VALIDATE | SDC_TOPOLOGY_EXTEND | SDC_ALLOW_PATH_ORDER_CHANGES));
				if (hr == S_OK)
				{
					hr = SetDisplayConfig(NumPathArrayElements, &PathInfoArray2[0], 0, NULL, (SDC_APPLY | SDC_TOPOLOGY_EXTEND | SDC_ALLOW_PATH_ORDER_CHANGES));
				}
			}

		}
	}

	return true;
}
bool MonitorHandler::isDSCEnabled()
{
	HRESULT hr = S_OK;
	UINT32 NumPathArrayElements = 0;
	UINT32 NumModeInfoArrayElements = 0;
	hr = GetDisplayConfigBufferSizes((QDC_ONLY_ACTIVE_PATHS), &NumPathArrayElements, &NumModeInfoArrayElements);
	std::vector<DISPLAYCONFIG_PATH_INFO> PathInfoArray2(NumPathArrayElements);
	DISPLAYCONFIG_PATH_INFO pathInfoHDMI = {};
	pathInfoHDMI.targetInfo.outputTechnology = DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER;
	std::vector<DISPLAYCONFIG_MODE_INFO> ModeInfoArray2(NumModeInfoArrayElements);
	hr = QueryDisplayConfig((QDC_ONLY_ACTIVE_PATHS), &NumPathArrayElements, &PathInfoArray2[0], &NumModeInfoArrayElements, &ModeInfoArray2[0], NULL);

	if (hr == S_OK)
	{
		for (UINT32 i = 0; i < NumPathArrayElements; i++)
		{
			if (PathInfoArray2[i].targetInfo.refreshRate.Numerator > MAX_REFRESH_RATE_DSC)
			{
				return true;
			}
		}
	}
	return false;
}

bool MonitorHandler::isSingleDisplayHDMI()
{
	HRESULT hr = S_OK;
	UINT32 NumPathArrayElements = 0;
	UINT32 NumModeInfoArrayElements = 0;
	hr = GetDisplayConfigBufferSizes((QDC_ONLY_ACTIVE_PATHS), &NumPathArrayElements, &NumModeInfoArrayElements);
	std::vector<DISPLAYCONFIG_PATH_INFO> PathInfoArray2(NumPathArrayElements);
	DISPLAYCONFIG_PATH_INFO pathInfoHDMI = {};
	pathInfoHDMI.targetInfo.outputTechnology = DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER;
	std::vector<DISPLAYCONFIG_MODE_INFO> ModeInfoArray2(NumModeInfoArrayElements);
	hr = QueryDisplayConfig((QDC_ONLY_ACTIVE_PATHS), &NumPathArrayElements, &PathInfoArray2[0], &NumModeInfoArrayElements, &ModeInfoArray2[0], NULL);

	if (hr == S_OK && NumPathArrayElements == 1 && PathInfoArray2[0].targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI)
	{
		return true;
	}
	return false;
}

int MonitorHandler::getActiveMonitorCount()
{
	UINT32 NumPathArrayElements = 0;
	UINT32 NumModeInfoArrayElements = 0;
	HRESULT hr = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &NumPathArrayElements, &NumModeInfoArrayElements);
	return NumPathArrayElements;
}