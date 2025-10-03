#include "framework.h"
#include "SerialHandler.h"
#define PI_VID L"0525"
#define PI_PID L"a4a7"

// GIP Polling Command
#define RASPBERRY_PI_GIP_GET_PWR_STATUS 0xb3

// Power Status Responses
#define PWR_STATUS_PI 0xef
#define PWR_STATUS_OTHER 0xaf

DWORD WINAPI SerialThread(LPVOID lpParam) {
	std::wstring* comPath = (std::wstring*)lpParam;
	HANDLE hSerial = CreateFileW(comPath->c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		hSerial = NULL;
		return 1;
	}

	DCB dcb = { 0 };
	dcb.DCBlength = sizeof(dcb);
	GetCommState(hSerial, &dcb);
	dcb.BaudRate = CBR_9600;
	SetCommState(hSerial, &dcb);
	HANDLE hEvents[2] = { NULL };

	HANDLE hShutdownEvent = CreateEventW(NULL, FALSE, FALSE, L"CECShutdownEvent");
	if (hShutdownEvent == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		hShutdownEvent = OpenEventW(SYNCHRONIZE, FALSE, L"CECShutdownEvent");
		if (hShutdownEvent)
		{
			ResetEvent(hShutdownEvent);
		}
	}
	HANDLE hNewDeviceEvent = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"NewDeviceEvent");
	if (hNewDeviceEvent == NULL && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		hNewDeviceEvent = CreateEventW(NULL, FALSE, FALSE, L"NewDeviceEvent");
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

	if (hShutdownEvent)
	{
		hEvents[0] = hShutdownEvent;
	}
	if (hNewDeviceEvent)
	{
		hEvents[1] = hNewDeviceEvent;
	}

	int pwrStatus = 0;
	int cmd = RASPBERRY_PI_GIP_GET_PWR_STATUS;
	bool serialEvent = false;
	DWORD dwWaitResult = WAIT_TIMEOUT;
	while (dwWaitResult <= (ARRAYSIZE(hEvents)) || dwWaitResult == WAIT_TIMEOUT) {
		dwWaitResult = WaitForMultipleObjects(ARRAYSIZE(hEvents), hEvents, FALSE, 50);
		switch (dwWaitResult)
		{
		case 0:
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
			if (hCECPowerOnEventSerial)
			{
				CloseHandle(hCECPowerOnEventSerial);
				hCECPowerOnEventSerial = NULL;
			}
			return 0;
		}
		case 1:
		{
			if (hSerial)
			{
				CloseHandle(hSerial);
			}
			hSerial = NULL;
			if (hEvents[1])
			{
				ResetEvent((hEvents[1]));
			}
			if (comPath->c_str() != L"")
			{
				hSerial = CreateFileW(comPath->c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
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
				serialEvent = false;
			}
			break;
		}
		case WAIT_TIMEOUT:
		{
			if (hSerial && !WriteFile(hSerial, &cmd, sizeof(cmd), NULL, NULL))
			{
				CloseHandle(hSerial);
				hSerial = NULL;
			}
			if (hSerial && !ReadFile(hSerial, &pwrStatus, sizeof(pwrStatus), NULL, NULL))
			{
				CloseHandle(hSerial);
				hSerial = NULL;
			}
			if (!serialEvent && pwrStatus == PWR_STATUS_PI && hCECPowerOnEventSerial && WaitForSingleObject(hCECPowerOnEventSerial, 1) != WAIT_OBJECT_0)
			{
				SetEvent(hCECPowerOnEventSerial);
				serialEvent = true;
			}
			break;
		}
		}
	}
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