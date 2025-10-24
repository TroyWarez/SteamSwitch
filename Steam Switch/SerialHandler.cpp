#include "framework.h"
#include "SerialHandler.h"

#define PI_VID L"0525"
#define PI_PID L"a4a7"

// GIP Polling Command
#define RASPBERRY_PI_GIP_POLL 0xaf
#define RASPBERRY_PI_GIP_SYNC 0xb0
#define RASPBERRY_PI_GIP_CLEAR 0xb1
#define RASPBERRY_PI_GIP_LOCK 0xb2
#define RASPBERRY_PI_GIP_GET_PWR_STATUS 0xb3
#define RASPBERRY_PI_CLEAR_NEXT_SYNCED_CONTROLLER 0xb4
#define NBOFEVENTS 7

// Power Status Responses
#define PWR_STATUS_PI 0xef
#define PWR_STATUS_OTHER 0xaf


extern DWORD32 controllerCount;

BOOL ReadADoubleWord32(HANDLE hComm, DWORD32* lpDW32)
{
	OVERLAPPED osRead = { 0 };
	DWORD dwRead = 0;
	DWORD dwRes = 0;
	BOOL fRes = FALSE;
	HANDLE hEvents[2] = { NULL };
	if (hComm)
	{
		// Create this read operation's OVERLAPPED structure's hEvent.
		osRead.hEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"SerialReadEvent");
		if (osRead.hEvent == NULL)
		{
			return FALSE;
		}
		hEvents[0] = osRead.hEvent;
		hEvents[1] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ShutdownEvent");

		if (hEvents[1] == NULL)
			// error creating overlapped event handle
			return FALSE;


		// Issue read.
		if (!ReadFile(hComm, lpDW32, sizeof(DWORD32), &dwRead, &osRead)) {
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
		CloseHandle(osRead.hEvent);
		CloseHandle(hEvents[1]);
	}
	return fRes;
}

BOOL WriteADoubleWord32(HANDLE hComm, DWORD32* lpDW32)
{
	OVERLAPPED osWrite = { 0 };
	DWORD dwWrite = 0;
	DWORD dwRes = 0;
	BOOL fRes = FALSE;
	HANDLE hEvents[2] = { NULL };
	if (hComm)
	{
		// Create this write operation's OVERLAPPED structure's hEvent.
		osWrite.hEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"SerialWriteEvent");

		if (osWrite.hEvent == NULL)
			// error creating overlapped event handle
			return FALSE;

		hEvents[0] = osWrite.hEvent;
		hEvents[1] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ShutdownEvent");
		if (hEvents[1] == NULL)
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

		PurgeComm(hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);
	}
	return fRes;
}
DWORD WINAPI SerialThread(LPVOID lpParam) {
	std::wstring* comPath = (std::wstring*)lpParam;
	HANDLE hSerial = CreateFileW(comPath->c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		hSerial = NULL;
	}
	else
	{
		DCB dcb = { 0 };
		dcb.DCBlength = sizeof(dcb);
		GetCommState(hSerial, &dcb);
		dcb.BaudRate = CBR_9600;
		SetCommState(hSerial, &dcb);
		PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
	}

	HANDLE hEvents[NBOFEVENTS] = { NULL };
	hEvents[0] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"CECShutdownEvent");
	if (hEvents[0] == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[0] = CreateEventW(NULL, FALSE, FALSE, L"CECShutdownEvent");
	}

	hEvents[1] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"SyncEvent");
	if (hEvents[1] == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[1] = CreateEventW(NULL, FALSE, FALSE, L"SyncEvent");
	}

	HANDLE hReadEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"SerialReadEvent");
	if (hReadEvent == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hReadEvent = CreateEventW(NULL, FALSE, FALSE, L"SerialReadEvent");
	}
	if (hReadEvent)
	{
		ResetEvent(hReadEvent);
	}

	HANDLE hWriteEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"SerialWriteEvent");
	if (hWriteEvent == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hWriteEvent = CreateEventW(NULL, FALSE, FALSE, L"SerialWriteEvent");
	}
	if (hWriteEvent)
	{
		ResetEvent(hWriteEvent);
	}

	HANDLE hFinshedSyncEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"FinshedSyncEvent");
	if (hFinshedSyncEvent == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hFinshedSyncEvent = CreateEventW(NULL, FALSE, FALSE, L"FinshedSyncEvent");
	}
	if (hFinshedSyncEvent)
	{
		ResetEvent(hFinshedSyncEvent);
	}

	hEvents[2] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ClearSingleEvent");
	if (hEvents[2] == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[2] = CreateEventW(NULL, FALSE, FALSE, L"ClearSingleEvent");
	}

	HANDLE hFinshedClearSingleEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"FinshedClearSingleEvent");
	if (hFinshedClearSingleEvent == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hFinshedClearSingleEvent = CreateEventW(NULL, FALSE, FALSE, L"FinshedClearSingleEvent");
	}

	hEvents[3] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ClearAllEvent");
	if (hEvents[3] == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[3] = CreateEventW(NULL, FALSE, FALSE, L"ClearAllEvent");
	}

	HANDLE hFinshedClearAllEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"FinshedClearAllEvent");
	if (hFinshedClearAllEvent == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hFinshedClearAllEvent = CreateEventW(NULL, FALSE, FALSE, L"FinshedClearAllEvent");
	}

	hEvents[4] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"LockDeviceEvent");
	if (hEvents[4] == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[4] = CreateEventW(NULL, FALSE, FALSE, L"LockDeviceEvent");
	}

	HANDLE hFinshedLockDeviceEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"FinshedLockDeviceEvent");
	if (hFinshedLockDeviceEvent == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hFinshedLockDeviceEvent = CreateEventW(NULL, FALSE, FALSE, L"FinshedLockDeviceEvent");
	}

	HANDLE hShutdownEvent = CreateEventW(NULL, FALSE, FALSE, L"CECShutdownEvent");
	if (hShutdownEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hShutdownEvent = OpenEventW(SYNCHRONIZE, FALSE, L"CECShutdownEvent");
		if (hShutdownEvent)
		{
			ResetEvent(hShutdownEvent);
		}
	}

	hEvents[5] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"NewDeviceEvent");
	if (hEvents[5] == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[5] = CreateEventW(NULL, FALSE, FALSE, L"NewDeviceEvent");
	}

	hEvents[6] = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ResumedFromSleep");
	if (hEvents[6] == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hEvents[6] = CreateEventW(NULL, FALSE, FALSE, L"ResumedFromSleep");
	}

	HANDLE hCECPowerOnEventSerial = CreateEventW(NULL, FALSE, FALSE, L"CECPowerOnEventSerial");
	if (hCECPowerOnEventSerial == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hCECPowerOnEventSerial = OpenEventW(SYNCHRONIZE, FALSE, L"CECPowerOnEventSerial");
		if (hCECPowerOnEventSerial)
		{
			ResetEvent(hCECPowerOnEventSerial);
		}
	}

	DWORD32 lastControllerCount = 0;
	DWORD32 cmd = RASPBERRY_PI_GIP_GET_PWR_STATUS;
	DWORD32 pwrStatus = PWR_STATUS_OTHER;
	DWORD dwWaitResult = 0;
	DWORD currentTime = 0;
	DWORD endTime = 0;
	BOOL readPwrStatus = FALSE;
	dwWaitResult = WAIT_TIMEOUT;

	if (!WriteADoubleWord32(hSerial, &cmd) ||
		!ReadADoubleWord32(hSerial, &pwrStatus))
	{
		CloseHandle(hSerial);
		pwrStatus = PWR_STATUS_OTHER;
		hSerial = NULL;
	}

	if ( pwrStatus == PWR_STATUS_PI &&
		hCECPowerOnEventSerial &&
		WaitForSingleObject(hCECPowerOnEventSerial, 1) == WAIT_TIMEOUT)
	{
		SetEvent(hCECPowerOnEventSerial);
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
					lastControllerCount = controllerCount;
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
					!ReadADoubleWord32(hSerial, &controllerCount))
				{
					CloseHandle(hSerial);
					controllerCount = -1;
					hSerial = NULL;
					break;
				}

				if (readPwrStatus == TRUE)
				{
					cmd = RASPBERRY_PI_GIP_GET_PWR_STATUS;
					if (!WriteADoubleWord32(hSerial, &cmd) ||
						!ReadADoubleWord32(hSerial, &pwrStatus))
					{
						CloseHandle(hSerial);
						controllerCount = -1;
						hSerial = NULL;
						break;
					}
					if (pwrStatus == PWR_STATUS_PI &&
						hCECPowerOnEventSerial &&
						WaitForSingleObject(hCECPowerOnEventSerial, 1) == WAIT_TIMEOUT)
					{
						SetEvent(hCECPowerOnEventSerial);
					}
					readPwrStatus = FALSE;
				}

				if (controllerCount > lastControllerCount && cmd == RASPBERRY_PI_GIP_SYNC)
				{
					if (hFinshedSyncEvent)
					{
						SetEvent(hFinshedSyncEvent);
					}
					cmd = RASPBERRY_PI_GIP_POLL;
				}
				if (controllerCount < lastControllerCount && cmd == RASPBERRY_PI_CLEAR_NEXT_SYNCED_CONTROLLER)
				{
					if (hFinshedClearSingleEvent)
					{
						SetEvent(hFinshedClearSingleEvent);
					}
					cmd = RASPBERRY_PI_GIP_POLL;
				}
				else if (controllerCount == 0 && cmd == RASPBERRY_PI_GIP_CLEAR)
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
				controllerCount = -1;
				Sleep(100);
				if (comPath && *(comPath) != L"")
				{
					hSerial = CreateFileW(comPath->c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
					if (hSerial == INVALID_HANDLE_VALUE)
					{
						hSerial = NULL;
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
				if (hEvents[i] != NULL)
				{
					CloseHandle(hEvents[i]);
				}
			}
			if (hSerial)
			{
				CloseHandle(hSerial);
				hSerial = NULL;
			}
			if (hWriteEvent)
			{
				CloseHandle(hWriteEvent);
				hWriteEvent = NULL;
			}
			if (hReadEvent)
			{
				CloseHandle(hReadEvent);
				hReadEvent = NULL;
			}
			return 0;
		}
		case 5: // hNewDeviceEvent
		{
			if (hSerial)
			{
				CloseHandle(hSerial);
			}
			hSerial = NULL;
			if (hEvents[5])
			{
				ResetEvent(hEvents[5]);
			}
			if (comPath && *(comPath) != L"")
			{
				hSerial = CreateFileW(comPath->c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
				if (hSerial == INVALID_HANDLE_VALUE)
				{
					hSerial = NULL;
				}
				else
				{
					DCB dcb = { 0 };
					dcb.DCBlength = sizeof(dcb);
					GetCommState(hSerial, &dcb);
					dcb.BaudRate = CBR_9600;
					SetCommState(hSerial, &dcb);
					cmd = RASPBERRY_PI_GIP_GET_PWR_STATUS;
					if (!WriteADoubleWord32(hSerial, &cmd) ||
						!ReadADoubleWord32(hSerial, &pwrStatus))
					{
						CloseHandle(hSerial);
						controllerCount = -1;
						hSerial = NULL;
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
	hSerial = NULL;
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
				if (hSerial == NULL)
				{
					hSerial = CreateThread(NULL, 0, SerialThread, &comPath, 0, NULL);
				}
				else if (GetExitCodeThread(hSerial, &exitCode) && exitCode != STILL_ACTIVE)
				{
					CloseHandle(hSerial);
					hSerial = CreateThread(NULL, 0, SerialThread, &comPath, 0, NULL);
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
	HDEVINFO hdevInfo = SetupDiGetClassDevs(ClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
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
		if (SetupDiEnumDeviceInterfaces(hdevInfo, NULL, ClassGuid, i, &deviceInterfaceData) == FALSE ||
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
			if (CM_Get_DevNode_Registry_PropertyW(deviceData.DevInst, CM_DRP_FRIENDLYNAME, NULL, NULL, &DataSize, 0) != CR_NO_SUCH_VALUE)
			{
				if (DataSize) {
					deviceName = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, DataSize * sizeof(WCHAR) + 2);
					if (CM_Get_DevNode_Registry_PropertyW(deviceData.DevInst, CM_DRP_FRIENDLYNAME, NULL, deviceName, &DataSize, 0) == CR_SUCCESS)
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
		if (deviceDetails == NULL)
		{
			SetupDiDestroyDeviceInfoList(hdevInfo);
			return FALSE;
		}
		deviceDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
		if (SetupDiGetDeviceInterfaceDetailW(hdevInfo, &deviceInterfaceData, deviceDetails, MAX_PATH * sizeof(WCHAR) + sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W), NULL, NULL) == TRUE)
		{
			std::wstring DevicePath = deviceDetails->DevicePath;
			HeapFree(GetProcessHeap(), 0, deviceDetails);
			DevicePaths.push_back(DevicePath);
		}
	}

	SetupDiDestroyDeviceInfoList(hdevInfo);
	return TRUE;
}