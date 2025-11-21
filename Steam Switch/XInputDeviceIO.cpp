// cl.exe xbox_test.c /link setupapi.lib user32.lib
#include "XInputDeviceIO.h"
#include "framework.h"

#include <cstdio>
void xbox_init()
{
	DWORD count = 0;
	std::array<HANDLE, XBOX_MAX_CONTROLLERS> windowsDir = { nullptr };

	HDEVINFO dev = SetupDiGetClassDevsW(&xbox_guid, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (dev != INVALID_HANDLE_VALUE)
	{
		SP_DEVICE_INTERFACE_DATA idata;
		idata.cbSize = sizeof(idata);

		DWORD index = 0;
		while (SetupDiEnumDeviceInterfaces(dev, nullptr, &xbox_guid, index, &idata))
		{
			DWORD size = 0;
			SetupDiGetDeviceInterfaceDetailW(dev, &idata, nullptr, 0, &size, nullptr);

			PSP_DEVICE_INTERFACE_DETAIL_DATA_W detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)LocalAlloc(LMEM_FIXED, size);
			if (detail != nullptr)
			{
				detail->cbSize = sizeof(*detail); // yes, sizeof of structure, not allocated memory

				SP_DEVINFO_DATA data;
				data.cbSize = sizeof(data);

				if (SetupDiGetDeviceInterfaceDetailW(dev, &idata, detail, size, &size, &data))
				{
					xbox_connect(detail->DevicePath);
				}
				LocalFree(detail);
			}
			index++;
		}

		SetupDiDestroyDeviceInfoList(dev);
	}
}

DWORD xbox_connect(LPWSTR path)
{
	for (DWORD i = 0; i < XBOX_MAX_CONTROLLERS; i++)
	{
		// yes, _wcsicmp, because SetupDi* functions and WM_DEVICECHANGE message provides different case paths...
		if (xbox_devices[i].handle != nullptr && _wcsicmp(xbox_devices[i].path, path) == 0)
		{
			return i;
		}
	}

	HANDLE handle = CreateFileW(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	for (DWORD i = 0; i < XBOX_MAX_CONTROLLERS; i++)
	{
		if (xbox_devices[i].handle == nullptr)
		{
			xbox_devices[i].handle = handle;
			//wcsncpy(xbox_devices[i].path, path, MAX_PATH);
			//printf("[%u] Connected\n", i);
			return i;
		}
	}

	return -1;
}

DWORD xbox_disconnect(LPWSTR path)
{
	for (DWORD i = 0; i < XBOX_MAX_CONTROLLERS; i++)
	{
		if (xbox_devices[i].handle != nullptr && _wcsicmp(xbox_devices[i].path, path) == 0)
		{
			CloseHandle(xbox_devices[i].handle);
			xbox_devices[i].handle = nullptr;
			return i;
		}
	}

	return -1;
}

DWORD xbox_get_caps(DWORD index, xbox_caps* caps)
{
	if (index >= XBOX_MAX_CONTROLLERS || xbox_devices[index].handle == nullptr)
	{
		return -1;
	}
	std::array<BYTE, 3> in = { 0x01, 0x01, 0x00 };
	std::array<BYTE, 24> out = { 0x00 };
	DWORD size = 0;
	if (!DeviceIoControl(xbox_devices[index].handle, 0x8000e004, in.data(), static_cast<DWORD>(in.size()), out.data(), static_cast<DWORD>(out.size()), &size, nullptr) || size != sizeof(out))
	{
		// NOTE: could check GetLastError() here, if it is ERROR_DEVICE_NOT_CONNECTED - that means disconnect
		return -1;
	}

	caps->type = out[2];
	caps->subtype = out[3];
	caps->flags = 4; // yes, always 4
	caps->buttons = *(WORD*)(out.data() + 4);
	caps->left_trigger = out[6];
	caps->right_trigger = out[7];
	caps->left_thumb_x = *(SHORT*)(out.data() + 8);
	caps->left_thumb_y = *(SHORT*)(out.data() + 10);
	caps->right_thumb_x = *(SHORT*)(out.data() + 12);
	caps->right_thumb_y = *(SHORT*)(out.data() + 14);
	caps->low_freq = out[22];
	caps->high_freq = out[23];
	return 0;
}

DWORD xbox_get_battery(DWORD index, xbox_battery* bat)
{
	if (index >= XBOX_MAX_CONTROLLERS || xbox_devices[index].handle == nullptr)
	{
		return -1;
	}
	std::array<BYTE, 4> in = { 0x02, 0x01, 0x00, 0x00 };
	std::array<BYTE, 4> out = { 0x00 };
	DWORD size = 0;
	if (!DeviceIoControl(xbox_devices[index].handle, 0x8000e018, in.data(), static_cast<DWORD>(in.size()), out.data(), static_cast<DWORD>(out.size()), &size, nullptr) || size != sizeof(out))
	{
		// NOTE: could check GetLastError() here, if it is ERROR_DEVICE_NOT_CONNECTED - that means disconnect
		return -1;
	}

	bat->type = out[2];
	bat->level = out[3];
	return 0;
}


DWORD xbox_get(DWORD index, XINPUT_STATE* state)
{
	if (index >= XBOX_MAX_CONTROLLERS || xbox_devices[index].handle == nullptr)
	{
		return -1;
	}
	std::array<BYTE, 3> in = { 0x01, 0x01, 0x00 };
	std::array<BYTE, 29> out = { 0x00 };

	DWORD size = 0;
	DWORD dwResult = ERROR_SUCCESS;
	if (!DeviceIoControl(xbox_devices[index].handle, 0x8000e00c, in.data(), static_cast<DWORD>(in.size()), out.data(), static_cast<DWORD>(out.size()), &size, nullptr) || size != sizeof(out))
	{
		// NOTE: could check GetLastError() here, if it is ERROR_DEVICE_NOT_CONNECTED - that means disconnect
		return XInputGetState(index, state);
	}
	dwResult = XInputGetState(index, state);
	WORD Buttons = *(WORD*)(out.data() + 11);
	if (Buttons & XBOX_GUIDE && state)
	{
		state->Gamepad.wButtons = state->Gamepad.wButtons | XBOX_GUIDE;
	}
	return ERROR_SUCCESS;
}

DWORD xbox_set(DWORD index, BYTE low_freq, BYTE high_freq)
{
	if (index >= XBOX_MAX_CONTROLLERS || xbox_devices[index].handle == nullptr)
	{
		return -1;
	}

	std::array<BYTE, 5> in = { 0, 0, low_freq, high_freq, 2 };
	if (!DeviceIoControl(xbox_devices[index].handle, 0x8000a010, in.data(), static_cast<DWORD>(in.size()), nullptr, 0, nullptr, nullptr))
	{
		// NOTE: could check GetLastError() here, if it is ERROR_DEVICE_NOT_CONNECTED - that means disconnect
		return -1;
	}
	return 0;
}


/// example code

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_DEVICECHANGE:
	{
		DEV_BROADCAST_HDR* hdr = (DEV_BROADCAST_HDR*)lparam;
		if (hdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
		{
			DEV_BROADCAST_DEVICEINTERFACE_W* dif = (DEV_BROADCAST_DEVICEINTERFACE_W*)hdr;
			if (wparam == DBT_DEVICEARRIVAL)
			{
				DWORD index = xbox_connect(dif->dbcc_name);

				xbox_caps caps;
				xbox_battery bat;
				if (xbox_get_caps(index, &caps) == 0 && xbox_get_battery(index, &bat) == 0)
				{
					printf("[%u] Type=%u SubType=%u ButtonsMask=%04x BatteryType=%u BatteryLevel=%u\n", index, caps.type, caps.subtype, caps.buttons, bat.type, bat.level);
				}
			}
			else if (wparam == DBT_DEVICEREMOVECOMPLETE)
			{
				DWORD index = xbox_disconnect(dif->dbcc_name);
				printf("[%u] Disconnected\n", index);
			}
		}
		return 0;
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	}

	return DefWindowProcW(window, message, wparam, lparam);
}
