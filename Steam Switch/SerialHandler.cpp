#include "framework.h"
#include "SerialHandler.h"
#include "SteamHandler.h"

constexpr LPCWSTR PI_VID = L"0525";
constexpr LPCWSTR PI_PID = L"a4a7";

// GIP Polling Command
constexpr int RASPBERRY_PI_GIP_POLL = 0xaf;
constexpr int RASPBERRY_PI_GIP_SYNC = 0xb0;
constexpr int RASPBERRY_PI_GIP_CLEAR = 0xb1;
constexpr int RASPBERRY_PI_GIP_LOCK = 0xb2;
constexpr int RASPBERRY_PI_CLEAR_NEXT_SYNCED_CONTROLLER = 0xb4;
constexpr int NBOFEVENTS = 7;

// Power Status Responses
constexpr int PWR_STATUS_PI = 0xef;
constexpr int PWR_STATUS_OTHER = 0xaf;

extern SteamHandler* steamHandler;
extern GIPSerialData serialData;

static BOOL ReadADoubleWord32(HANDLE hComm, GIPSerialData* lpDW32)
{
	OVERLAPPED osRead = { 0 };
	DWORD dwRead = 0;
	DWORD dwRes = 0;
	BOOL fRes = FALSE;
	HANDLE hEvents[2] = { nullptr };
	if (hComm)
	{
		// Create this read operation's OVERLAPPED structure's hEvent.
		osRead.hEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"SerialReadEvent");
		if (osRead.hEvent == nullptr)
		{
			return FALSE;
		}
		hEvents[0] = osRead.hEvent;
		hEvents[1] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ShutdownEvent");

		if (hEvents[1] == nullptr)
			// error creating overlapped event handle
			return FALSE;


		// Issue read.
		if (!ReadFile(hComm, lpDW32, sizeof(GIPSerialData), &dwRead, &osRead)) {
			if (GetLastError() != ERROR_IO_PENDING) {
				// ReadFile failed, but isn't delayed. Report error and abort.
				fRes = FALSE;
			}
			else
				// Read is pending.
				dwRes = WaitForMultipleObjects(ARRAYSIZE(hEvents), hEvents, FALSE, INFINITE);
			switch (dwRes)
			{
				// OVERLAPPED structure's event has been signaled. 
			case 0:
				if (!GetOverlappedResult(hComm, &osRead, &dwRead, FALSE))
					fRes = FALSE;
				else
					// Read operation completed successfully.
					fRes = TRUE;
				break;

			default:
				// An error has occurred in WaitForSingleObject.
				// This usually indicates a problem with the
			   // OVERLAPPED structure's event handle.
				fRes = FALSE;
				break;
			}
		}
		else {
			// ReadFile completed immediately.
			fRes = TRUE;
		}
		PurgeComm(hComm, PURGE_TXCLEAR);
		CloseHandle(osRead.hEvent);
		CloseHandle(hEvents[1]);
	}
	return fRes;
}

static BOOL WriteADoubleWord32(HANDLE hComm, DWORD32* lpDW32)
{
	OVERLAPPED osWrite = { 0 };
	DWORD dwWrite = 0;
	DWORD dwRes = 0;
	BOOL fRes = FALSE;
	HANDLE hEvents[2] = { nullptr };
	if (hComm)
	{
		// Create this write operation's OVERLAPPED structure's hEvent.
		osWrite.hEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"SerialWriteEvent");

		if (osWrite.hEvent == nullptr)
			// error creating overlapped event handle
			return FALSE;

		hEvents[0] = osWrite.hEvent;
		hEvents[1] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ShutdownEvent");
		if (hEvents[1] == nullptr)
			// error creating overlapped event handle
			return FALSE;
		// Issue write.
		if (!WriteFile(hComm, lpDW32, sizeof(DWORD32), &dwWrite, &osWrite)) {
			if (GetLastError() != ERROR_IO_PENDING) {
				// WriteFile failed, but isn't delayed. Report error and abort.
				fRes = FALSE;
			}
			else
				// Write is pending.
				dwRes = WaitForMultipleObjects(ARRAYSIZE(hEvents), hEvents, FALSE, INFINITE);
			switch (dwRes)
			{
				// OVERLAPPED structure's event has been signaled. 
			case 0:
				if (!GetOverlappedResult(hComm, &osWrite, &dwWrite, FALSE))
					fRes = FALSE;
				else
					// Write operation completed successfully.
					fRes = TRUE;
				break;
			default:
				// An error has occurred in WaitForSingleObject.
				// This usually indicates a problem with the
			   // OVERLAPPED structure's event handle.
				fRes = FALSE;
				break;
			}
		}
		else {
			// WriteFile completed immediately.
			fRes = TRUE;
		}
		CloseHandle(osWrite.hEvent);
		CloseHandle(hEvents[1]);

		PurgeComm(hComm, PURGE_RXCLEAR);
	}
	return fRes;
}
static DWORD WINAPI SerialThread(LPVOID lpParam) {
	std::wstring* comPath = (std::wstring*)lpParam;
	HANDLE hSerial = CreateFileW(comPath->c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		hSerial = nullptr;
	}
	else
	{
		DCB dcb = { 0 };
		dcb.DCBlength = sizeof(dcb);
		GetCommState(hSerial, &dcb);
		dcb.BaudRate = CBR_9600;
		SetCommState(hSerial, &dcb);
		PurgeComm(hSerial, PURGE_RXABORT | PURGE_RXABORT );
	}

	HANDLE hEvents[NBOFEVENTS] = { nullptr };
	hEvents[0] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ShutdownEvent");
	if (hEvents[0] == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[0] = CreateEventW(nullptr, FALSE, FALSE, L"ShutdownEvent");
	}

	hEvents[1] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"SyncEvent");
	if (hEvents[1] == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[1] = CreateEventW(nullptr, FALSE, FALSE, L"SyncEvent");
	}

	HANDLE hReadEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"SerialReadEvent");
	if (hReadEvent == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hReadEvent = CreateEventW(nullptr, FALSE, FALSE, L"SerialReadEvent");
	}
	if (hReadEvent)
	{
		ResetEvent(hReadEvent);
	}

	HANDLE hWriteEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"SerialWriteEvent");
	if (hWriteEvent == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hWriteEvent = CreateEventW(nullptr, FALSE, FALSE, L"SerialWriteEvent");
	}
	if (hWriteEvent)
	{
		ResetEvent(hWriteEvent);
	}

	HANDLE hFinshedSyncEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"FinshedSyncEvent");
	if (hFinshedSyncEvent == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hFinshedSyncEvent = CreateEventW(nullptr, FALSE, FALSE, L"FinshedSyncEvent");
	}
	if (hFinshedSyncEvent)
	{
		ResetEvent(hFinshedSyncEvent);
	}

	hEvents[2] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ClearSingleEvent");
	if (hEvents[2] == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[2] = CreateEventW(nullptr, FALSE, FALSE, L"ClearSingleEvent");
	}

	HANDLE hFinshedClearSingleEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"FinshedClearSingleEvent");
	if (hFinshedClearSingleEvent == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hFinshedClearSingleEvent = CreateEventW(nullptr, FALSE, FALSE, L"FinshedClearSingleEvent");
	}

	hEvents[3] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ClearAllEvent");
	if (hEvents[3] == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[3] = CreateEventW(nullptr, FALSE, FALSE, L"ClearAllEvent");
	}

	HANDLE hFinshedClearAllEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"FinshedClearAllEvent");
	if (hFinshedClearAllEvent == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hFinshedClearAllEvent = CreateEventW(nullptr, FALSE, FALSE, L"FinshedClearAllEvent");
	}

	hEvents[4] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"LockDeviceEvent");
	if (hEvents[4] == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[4] = CreateEventW(nullptr, FALSE, FALSE, L"LockDeviceEvent");
	}

	HANDLE hFinshedLockDeviceEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"FinshedLockDeviceEvent");
	if (hFinshedLockDeviceEvent == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hFinshedLockDeviceEvent = CreateEventW(nullptr, FALSE, FALSE, L"FinshedLockDeviceEvent");
	}

	hEvents[5] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"NewDeviceEvent");
	if (hEvents[5] == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[5] = CreateEventW(nullptr, FALSE, FALSE, L"NewDeviceEvent");
	}

	hEvents[6] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ResumedFromSleep");
	if (hEvents[6] == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[6] = CreateEventW(nullptr, FALSE, FALSE, L"ResumedFromSleep");
	}

	HANDLE hCECPowerOnEventSerial = CreateEventW(nullptr, FALSE, FALSE, L"CECPowerOnEventSerial");
	if (hCECPowerOnEventSerial == nullptr && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hCECPowerOnEventSerial = OpenEventW(SYNCHRONIZE, FALSE, L"CECPowerOnEventSerial");
		if (hCECPowerOnEventSerial)
		{
			ResetEvent(hCECPowerOnEventSerial);
		}
	}

	DWORD32 lastControllerCount = 0;
 	DWORD32 cmd = RASPBERRY_PI_GIP_POLL;
// 	DWORD32 pwrStatus = PWR_STATUS_OTHER;
	DWORD dwWaitResult = 0;
	DWORD currentTime = 0;
	DWORD endTime = 0;
	BOOL readPwrStatus = FALSE;
	dwWaitResult = WAIT_TIMEOUT;

	if (!WriteADoubleWord32(hSerial, &cmd) ||
		!ReadADoubleWord32(hSerial, &serialData))
	{
		CloseHandle(hSerial);
		serialData.pwrStatus = PWR_STATUS_OTHER;
		hSerial = nullptr;
	}

	if (steamHandler &&
		steamHandler->monHandler &&
		serialData.pwrStatus == PWR_STATUS_PI &&
		hCECPowerOnEventSerial &&
		WaitForSingleObject(hCECPowerOnEventSerial, 1) == WAIT_TIMEOUT)
	{
		if (!steamHandler->monHandler->isDSCEnabled())
		{
			if (steamHandler->monHandler->isSingleDisplayHDMI() &&
				WaitForSingleObject(steamHandler->monHandler->hCECPowerOnEvent, 1) == WAIT_TIMEOUT)
			{
				SetEvent(steamHandler->monHandler->hCECPowerOnEvent);
			}
			else if (!steamHandler->monHandler->isSingleDisplayHDMI())
			{
				ShellExecuteW(GetDesktopWindow(), L"open", L"steam://open/bigpicture", nullptr, nullptr, SW_SHOW);
			}
		}

	}
	cmd = RASPBERRY_PI_GIP_POLL;
	while (dwWaitResult == WAIT_TIMEOUT) {
		dwWaitResult = WaitForMultipleObjects(ARRAYSIZE(hEvents), hEvents, FALSE, 50);
		currentTime = timeGetTime();
		switch (dwWaitResult)
		{
		case 1: // hSyncEvent
		{
			if (hEvents[1] && dwWaitResult == 1)
			{
				ResetEvent(hEvents[1]);
				cmd = RASPBERRY_PI_GIP_SYNC;
			}
			break;
		}
		case 2: // hClearSingleEvent
		{
			if (hEvents[2] && dwWaitResult == 2)
			{
				ResetEvent(hEvents[2]);
				cmd = RASPBERRY_PI_CLEAR_NEXT_SYNCED_CONTROLLER;
			}
			break;
		}
		case 3: // hClearAllEvent
		{
			if (hEvents[3] && dwWaitResult == 3)
			{
				ResetEvent(hEvents[3]);
				cmd = RASPBERRY_PI_GIP_CLEAR;
			}
			break;
		}
		case 4: // hLockDeviceEvent
		{
			if (hEvents[4] && dwWaitResult == 4)
			{
				ResetEvent(hEvents[4]);
				cmd = RASPBERRY_PI_GIP_LOCK;
			}
			break;
		}
		case WAIT_TIMEOUT:
		{
			if (hSerial)
			{
				if (dwWaitResult == 1 ||
					dwWaitResult == 2 ||
					dwWaitResult == 3)
				{
					lastControllerCount = serialData.controllerCount;
					endTime = timeGetTime();
					endTime = endTime + 30000;
					dwWaitResult = WAIT_TIMEOUT;
				}

				if (endTime < currentTime && endTime != 0)
				{
					cmd = RASPBERRY_PI_GIP_POLL;
					endTime = 0;
				}

				if (!WriteADoubleWord32(hSerial, &cmd) ||
					!ReadADoubleWord32(hSerial, &serialData))
				{
					CloseHandle(hSerial);
					serialData.controllerCount = -1;
					serialData.pwrStatus = PWR_STATUS_OTHER;
					hSerial = nullptr;
					break;
				}

				if (readPwrStatus == TRUE)
				{
					if (steamHandler &&
						steamHandler->monHandler &&
						!steamHandler->monHandler->isDSCEnabled() &&
						serialData.pwrStatus == PWR_STATUS_PI &&
						hCECPowerOnEventSerial &&
						WaitForSingleObject(hCECPowerOnEventSerial, 1) == WAIT_TIMEOUT)
					{
						ShellExecuteW(GetDesktopWindow(), L"open", L"steam://open/bigpicture", nullptr, nullptr, SW_SHOW);
						SetEvent(hCECPowerOnEventSerial);
					}
					readPwrStatus = FALSE;
				}

				if (serialData.controllerCount > lastControllerCount && cmd == RASPBERRY_PI_GIP_SYNC)
				{
					if (hFinshedSyncEvent)
					{
						SetEvent(hFinshedSyncEvent);
					}
					cmd = RASPBERRY_PI_GIP_POLL;
				}
				if (serialData.controllerCount < lastControllerCount && cmd == RASPBERRY_PI_CLEAR_NEXT_SYNCED_CONTROLLER ||
					serialData.controllerCount == 0 && cmd == RASPBERRY_PI_CLEAR_NEXT_SYNCED_CONTROLLER)
				{
					if (hFinshedClearSingleEvent)
					{
						SetEvent(hFinshedClearSingleEvent);
					}
					cmd = RASPBERRY_PI_GIP_POLL;
				}
				else if (serialData.controllerCount == 0 && cmd == RASPBERRY_PI_GIP_CLEAR)
				{
					if (hFinshedClearAllEvent)
					{
						SetEvent(hFinshedClearAllEvent);
					}
					cmd = RASPBERRY_PI_GIP_POLL;
				}

				if (cmd == RASPBERRY_PI_GIP_LOCK)
				{
					if (hFinshedLockDeviceEvent)
					{
						SetEvent(hFinshedLockDeviceEvent);

					}
					if (hEvents[0])
					{
						SetEvent(hEvents[0]);
					}
				}
			}
			else
			{
				serialData.controllerCount = -1;
				serialData.pwrStatus = PWR_STATUS_OTHER;
				Sleep(100);
				if (comPath && *(comPath) != L"")
				{
					hSerial = CreateFileW(comPath->c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
					if (hSerial == INVALID_HANDLE_VALUE)
					{
						hSerial = nullptr;
					}
					else
					{
						DCB dcb = { 0 };
						dcb.DCBlength = sizeof(dcb);
						GetCommState(hSerial, &dcb);
						dcb.BaudRate = CBR_9600;
						SetCommState(hSerial, &dcb);
					}
				}
			}
			break;
		}
		case 0: // hShutdownEvent
		{
			for (size_t i = 0; i < ARRAYSIZE(hEvents); i++)
			{
				if (hEvents[i] != nullptr)
				{
					CloseHandle(hEvents[i]);
				}
			}
			if (hSerial)
			{
				CloseHandle(hSerial);
				hSerial = nullptr;
			}
			if (hWriteEvent)
			{
				CloseHandle(hWriteEvent);
				hWriteEvent = nullptr;
			}
			if (hReadEvent)
			{
				CloseHandle(hReadEvent);
				hReadEvent = nullptr;
			}
			//SetEvent(hF);
			return 0;
		}
		case 5: // hNewDeviceEvent
		{
			if (hSerial)
			{
				CloseHandle(hSerial);
			}
			hSerial = nullptr;
			if (hEvents[5])
			{
				ResetEvent(hEvents[5]);
			}
			if (comPath && *(comPath) != L"")
			{
				hSerial = CreateFileW(comPath->c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
				if (hSerial == INVALID_HANDLE_VALUE)
				{
					hSerial = nullptr;
				}
				else
				{
					DCB dcb = { 0 };
					dcb.DCBlength = sizeof(dcb);
					GetCommState(hSerial, &dcb);
					dcb.BaudRate = CBR_9600;
					SetCommState(hSerial, &dcb);
					cmd = RASPBERRY_PI_GIP_POLL;
					if (!WriteADoubleWord32(hSerial, &cmd) ||
						!ReadADoubleWord32(hSerial, &serialData))
					{
						CloseHandle(hSerial);
						serialData.controllerCount = -1;
						hSerial = nullptr;
						break;
					}
					cmd = RASPBERRY_PI_GIP_POLL;
				}
			}
			break;
		}
		case 6: // hResumedFromSleep
		{
			if (hEvents[6])
			{
				ResetEvent(hEvents[6]);
				readPwrStatus = TRUE;
			}
			break;
		}
		}
		dwWaitResult = WAIT_TIMEOUT;
	}
	return 1;
}
BOOL FindAllDevices(const GUID* ClassGuid, std::vector<std::wstring>& DevicePaths, std::vector<std::wstring>* DeviceNames);

SerialHandler::SerialHandler()
{
	hSerial = nullptr;
	ScanForSerialDevices();
}
SerialHandler::~SerialHandler()
{
	if (hSerial)
	{
		CloseHandle(hSerial);
	}
}
void SerialHandler::ScanForSerialDevices()
{
	std::vector<std::wstring> DevicePaths;
	std::vector<std::wstring> DeviceNames;
	FindAllDevices((LPGUID)&GUID_DEVINTERFACE_COMPORT, DevicePaths, &DeviceNames);
	for (size_t i = 0; i < DevicePaths.size(); i++)
	{
		if (DevicePaths[i].find(L"vid_" + std::wstring(PI_VID) + L"&pid_" + PI_PID) != std::wstring::npos)
		{
			devicePath = DevicePaths[i];
			if (DeviceNames[i].find(L"COM") != std::wstring::npos)
			{
				devicePort = DeviceNames[i].substr(DeviceNames[i].find(L"COM"), DeviceNames[i].find(L")"));
				devicePort = devicePort.substr(0, 5);
				comPath = L"\\\\.\\" + devicePort;
				DWORD exitCode = 0;
				if (hSerial == nullptr)
				{
					hSerial = CreateThread(nullptr, 0, SerialThread, &comPath, 0, nullptr);
				}
				else if (GetExitCodeThread(hSerial, &exitCode) && exitCode != STILL_ACTIVE)
				{
					CloseHandle(hSerial);
					hSerial = CreateThread(nullptr, 0, SerialThread, &comPath, 0, nullptr);
				}

			}
			break;
		}
	}
}
BOOL FindAllDevices(const GUID* ClassGuid, std::vector<std::wstring>& DevicePaths, std::vector<std::wstring>* DeviceNames)
{
	if (DevicePaths.size())
	{
		DevicePaths.clear();
	}
	PSP_DEVICE_INTERFACE_DETAIL_DATA_W deviceDetails = nullptr;
	WCHAR* deviceName = nullptr;
	HDEVINFO hdevInfo = SetupDiGetClassDevs(ClassGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hdevInfo == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	for (DWORD i = 0; ; i++)
	{
		deviceName = nullptr;
		SP_DEVICE_INTERFACE_DATA deviceInterfaceData = { 0 };
		SP_DEVINFO_DATA deviceData = { 0 };
		ULONG DataSize = 0;
		deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		deviceData.cbSize = sizeof(SP_DEVINFO_DATA);
		if (SetupDiEnumDeviceInterfaces(hdevInfo, nullptr, ClassGuid, i, &deviceInterfaceData) == FALSE ||
			SetupDiEnumDeviceInfo(hdevInfo, i, &deviceData) == FALSE)
		{
			if (GetLastError() != ERROR_NO_MORE_ITEMS) {
				SetupDiDestroyDeviceInfoList(hdevInfo);
				return FALSE;
			}
			else
			{
				break;
			}
		}
		if (DeviceNames != nullptr) {
			if (CM_Get_DevNode_Registry_PropertyW(deviceData.DevInst, CM_DRP_FRIENDLYNAME, nullptr, nullptr, &DataSize, 0) != CR_NO_SUCH_VALUE)
			{
				if (DataSize) {
					deviceName = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, DataSize * sizeof(WCHAR) + 2);
					if (CM_Get_DevNode_Registry_PropertyW(deviceData.DevInst, CM_DRP_FRIENDLYNAME, nullptr, deviceName, &DataSize, 0) == CR_SUCCESS)
					{
						if (deviceName != 0) {
							DeviceNames->push_back(deviceName);
						}
					}
					HeapFree(GetProcessHeap(), 0, deviceName);
				}
			}

		}
		deviceDetails = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)HeapAlloc(GetProcessHeap(), 0, MAX_PATH * sizeof(WCHAR) + sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W));
		if (deviceDetails == nullptr)
		{
			SetupDiDestroyDeviceInfoList(hdevInfo);
			return FALSE;
		}
		deviceDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
		if (SetupDiGetDeviceInterfaceDetailW(hdevInfo, &deviceInterfaceData, deviceDetails, MAX_PATH * sizeof(WCHAR) + sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W), nullptr, nullptr) == TRUE)
		{
			std::wstring DevicePath = deviceDetails->DevicePath;
			HeapFree(GetProcessHeap(), 0, deviceDetails);
			DevicePaths.push_back(DevicePath);
		}
	}

	SetupDiDestroyDeviceInfoList(hdevInfo);
	return TRUE;
}