// cl.exe xbox_test.c /link setupapi.lib user32.lib
#include "XInputDeviceIO.h"
#include "framework.h"

#include <stdio.h>
void xbox_init()
{
	DWORD count = 0;
	HANDLE new_handles[XBOX_MAX_CONTROLLERS];
	ZeroMemory(new_handles, sizeof(new_handles));

	HDEVINFO dev = SetupDiGetClassDevsW(&xbox_guid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (dev != INVALID_HANDLE_VALUE)
	{
		SP_DEVICE_INTERFACE_DATA idata;
		idata.cbSize = sizeof(idata);

		DWORD index = 0;
		while (SetupDiEnumDeviceInterfaces(dev, NULL, &xbox_guid, index, &idata))
		{
			DWORD size;
			SetupDiGetDeviceInterfaceDetailW(dev, &idata, NULL, 0, &size, NULL);

			PSP_DEVICE_INTERFACE_DETAIL_DATA_W detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)LocalAlloc(LMEM_FIXED, size);
			if (detail != NULL)
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

int xbox_connect(LPWSTR path)
{
	for (DWORD i = 0; i < XBOX_MAX_CONTROLLERS; i++)
	{
		// yes, _wcsicmp, because SetupDi* functions and WM_DEVICECHANGE message provides different case paths...
		if (xbox_devices[i].handle != NULL && _wcsicmp(xbox_devices[i].path, path) == 0)
		{
			return i;
		}
	}

	HANDLE handle = CreateFileW(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	for (DWORD i = 0; i < XBOX_MAX_CONTROLLERS; i++)
	{
		if (xbox_devices[i].handle == NULL)
		{
			xbox_devices[i].handle = handle;
			//wcsncpy(xbox_devices[i].path, path, MAX_PATH);
			//printf("[%u] Connected\n", i);
			return i;
		}
	}

	return -1;
}

int xbox_disconnect(LPWSTR path)
{
	for (DWORD i = 0; i < XBOX_MAX_CONTROLLERS; i++)
	{
		if (xbox_devices[i].handle != NULL && _wcsicmp(xbox_devices[i].path, path) == 0)
		{
			CloseHandle(xbox_devices[i].handle);
			xbox_devices[i].handle = NULL;
			return i;
		}
	}

	return -1;
}

int xbox_get_caps(DWORD index, xbox_caps* caps)
{
	if (index >= XBOX_MAX_CONTROLLERS || xbox_devices[index].handle == NULL)
	{
		return -1;
	}

	BYTE in[3] = { 0x01, 0x01, 0x00 };
	BYTE out[24];
	DWORD size;
	if (!DeviceIoControl(xbox_devices[index].handle, 0x8000e004, in, sizeof(in), out, sizeof(out), &size, NULL) || size != sizeof(out))
	{
		// NOTE: could check GetLastError() here, if it is ERROR_DEVICE_NOT_CONNECTED - that means disconnect
		return -1;
	}

	caps->type = out[2];
	caps->subtype = out[3];
	caps->flags = 4; // yes, always 4
	caps->buttons = *(WORD*)(out + 4);
	caps->left_trigger = out[6];
	caps->right_trigger = out[7];
	caps->left_thumb_x = *(SHORT*)(out + 8);
	caps->left_thumb_y = *(SHORT*)(out + 10);
	caps->right_thumb_x = *(SHORT*)(out + 12);
	caps->right_thumb_y = *(SHORT*)(out + 14);
	caps->low_freq = out[22];
	caps->high_freq = out[23];
	return 0;
}

int xbox_get_battery(DWORD index, xbox_battery* bat)
{
	if (index >= XBOX_MAX_CONTROLLERS || xbox_devices[index].handle == NULL)
	{
		return -1;
	}

	BYTE in[4] = { 0x02, 0x01, 0x00, 0x00 };
	BYTE out[4];
	DWORD size;
	if (!DeviceIoControl(xbox_devices[index].handle, 0x8000e018, in, sizeof(in), out, sizeof(out), &size, NULL) || size != sizeof(out))
	{
		// NOTE: could check GetLastError() here, if it is ERROR_DEVICE_NOT_CONNECTED - that means disconnect
		return -1;
	}

	bat->type = out[2];
	bat->level = out[3];
	return 0;
}


int xbox_get(DWORD index, XINPUT_STATE* state)
{
	if (index >= XBOX_MAX_CONTROLLERS || xbox_devices[index].handle == NULL)
	{
		return -1;
	}

	BYTE in[3] = { 0x01, 0x01, 0x00 };
	BYTE out[29];

	DWORD size;
	DWORD dwResult = ERROR_SUCCESS;
	if (!DeviceIoControl(xbox_devices[index].handle, 0x8000e00c, in, sizeof(in), out, sizeof(out), &size, NULL) || size != sizeof(out))
	{
		// NOTE: could check GetLastError() here, if it is ERROR_DEVICE_NOT_CONNECTED - that means disconnect
		
		return XInputGetState(index, state);
	}
	dwResult = XInputGetState(index, state);
	WORD Buttons = *(WORD*)(out + 11);
	if (Buttons & XBOX_GUIDE && state)
	{
		state->Gamepad.wButtons = state->Gamepad.wButtons | XBOX_GUIDE;
	}
	return ERROR_SUCCESS;
}


int xbox_set(DWORD index, BYTE low_freq, BYTE high_freq)
{
	if (index >= XBOX_MAX_CONTROLLERS || xbox_devices[index].handle == NULL)
	{
		return -1;
	}

	BYTE in[5] = { 0, 0, low_freq, high_freq, 2 };
	if (!DeviceIoControl(xbox_devices[index].handle, 0x8000a010, in, sizeof(in), NULL, 0, NULL, NULL))
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
