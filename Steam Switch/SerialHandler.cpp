#include "framework.h"
#include "SerialHandler.h"
#define PI_VID L"0525"
#define PI_PID L"a4a7"

DWORD WINAPI SerialThread(LPVOID lpParam) {
	std::wstring* comPath = (std::wstring*)lpParam;
	HANDLE hSerial = CreateFileW(comPath->c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		hSerial = NULL;
		return 1;
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

	HANDLE hIRPowerEvent = CreateEventW(NULL, FALSE, FALSE, L"IRPowerEvent");
	if (hIRPowerEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hIRPowerEvent = OpenEventW(SYNCHRONIZE, FALSE, L"IRPowerEvent");
		if (hIRPowerEvent)
		{
			ResetEvent(hIRPowerEvent);
		}
	}
	HANDLE hIRVolumeUpEvent = CreateEventW(NULL, FALSE, FALSE, L"IRVolumeUpEvent");
	if (hIRVolumeUpEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hIRVolumeUpEvent = OpenEventW(SYNCHRONIZE, FALSE, L"IRVolumeUpEvent");
		if (hIRVolumeUpEvent)
		{
			ResetEvent(hIRVolumeUpEvent);
		}
	}

	HANDLE hIRVolumeDownEvent = CreateEventW(NULL, FALSE, FALSE, L"IRVolumeDownEvent");
	if (hIRVolumeDownEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hIRVolumeDownEvent = OpenEventW(SYNCHRONIZE, FALSE, L"IRVolumeDownEvent");
		if (hIRVolumeDownEvent)
		{
			ResetEvent(hIRVolumeDownEvent);
		}
	}
	HANDLE hIRMuteEvent = CreateEventW(NULL, FALSE, FALSE, L"IRMuteEvent");
	if (hIRMuteEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hIRMuteEvent = OpenEventW(SYNCHRONIZE, FALSE, L"IRMuteEvent");
		if (hIRMuteEvent)
		{
			ResetEvent(hIRMuteEvent);
		}
	}
	std::vector<HANDLE> hEvents;

	if (!hShutdownEvent)
	{
		return 1;
	}
	hEvents.push_back(hShutdownEvent);
	hEvents.push_back(hIRPowerEvent);
	hEvents.push_back(hIRVolumeUpEvent);
	hEvents.push_back(hIRVolumeDownEvent);
	hEvents.push_back(hIRMuteEvent);



	UCHAR pwrStatus = 0;
	USHORT cmd = 0x00af;
	DWORD dwWaitResult = 0;
	while (dwWaitResult <= ((DWORD)hEvents.size() - 1) || dwWaitResult == WAIT_TIMEOUT) {
		dwWaitResult = WaitForMultipleObjects((DWORD)hEvents.size(), hEvents.data(), FALSE, 1);
		if (hSerial)
		{

			if (!ReadFile(hSerial, &pwrStatus, sizeof(pwrStatus), NULL, NULL))
			{
				CloseHandle(hSerial);
				return 1;
			}
			if (!WriteFile(hSerial, &cmd, sizeof(cmd), NULL, NULL))
			{
				CloseHandle(hSerial);
				return 2;
			}
		}
	}

// 	DWORD dwWaitResult = 0;
// 	while (dwWaitResult <= ((DWORD)hEvents.size() - 1)) {
// 		dwWaitResult = WaitForMultipleObjects((DWORD)hEvents.size(), hEvents.data(), FALSE, INFINITE);
// 		switch (dwWaitResult)
// 		{
// 		case 0: // hShutdownEvent
// 		{
// 			for (size_t i = 0; i < hEvents.size(); i++)
// 			{
// 				if (hEvents[i] != NULL)
// 				{
// 					CloseHandle(hEvents[i]);
// 				}
// 			}
// 			return 0;
// 		}
// 		case 1: // hIRVolumeUpEvent
// 		{
// 			if (hIRPowerEvent)
// 			{
// 				ResetEvent(hIRPowerEvent);
// 			}
// 			break;
// 		}
// 		case 2: // hIRVolumeUpEvent
// 		{
// 			if (hIRVolumeUpEvent)
// 			{
// 				ResetEvent(hIRVolumeUpEvent);
// 			}
// 			break;
// 		}
// 		}
// 	}
	return 0;
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
		TerminateThread(hSerial, 0);
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
				if (hSerial != NULL && GetExitCodeThread(hSerial, &exitCode) && exitCode == STILL_ACTIVE)
				{
					TerminateThread(hSerial, 0);
					CloseHandle(hSerial);
					hSerial = NULL;
				}
				hSerial = CreateThread(NULL, 0, SerialThread, &comPath, 0, NULL);
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