#include "framework.h"
#include "SerialHandler.h"
#define PI_VID L"0525"
#define PI_PID L"a4a7"

BOOL FindAllDevices(const GUID* ClassGuid, std::vector<std::wstring>& DevicePaths, std::vector<std::wstring>* DeviceNames);

SerialHandler::SerialHandler()
{
	hSerial = NULL;
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
			}
			hSerial = CreateFileW(comPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (hSerial == INVALID_HANDLE_VALUE)
			{
				hSerial = NULL;
			}
			break;
		}
	}

	UCHAR pwrStatus = 0;
	USHORT cmd = 0x00af;
 	while (hSerial != INVALID_HANDLE_VALUE)
	{
		if (hSerial)
		{
			
			if (!ReadFile(hSerial, &pwrStatus, sizeof(pwrStatus), NULL, NULL))
			{
				CloseHandle(hSerial);
				hSerial = CreateFileW(comPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			}
			if (!WriteFile(hSerial, &cmd, sizeof(cmd), NULL, NULL))
			{
				CloseHandle(hSerial);
				hSerial = CreateFileW(comPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			}
			Sleep(1);
		}

	}

}
SerialHandler::~SerialHandler()
{

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