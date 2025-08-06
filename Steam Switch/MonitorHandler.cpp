#include "MonitorHandler.h"
#include <iostream>
#include <cecloader.h>
#include <vector>
// It may be be possible to have two cec usb devices on the same cable

MonitorHandler::MonitorHandler(MonitorMode mode)
{
	cec_config.Clear();

	cec_config.clientVersion = CEC::LIBCEC_VERSION_CURRENT;
	cec_config.bActivateSource = 0;
	cec_config.bAutoPowerOn = 0;
	cec_config.iPhysicalAddress = 0;
	cec_config.bAutodetectAddress = 0;
	cec_config.bGetSettingsFromROM = 1;
	cec_config.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);

	HANDLE hConfigFile = CreateFileA("cecHDMI_Port.txt", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hConfigFile != INVALID_HANDLE_VALUE)
	{
		DWORD fileSize = GetFileSize(hConfigFile, NULL);
		if (fileSize == INVALID_FILE_SIZE || fileSize != 1)
		{
			// If the file is empty, write a default value
			cec_config.iHDMIPort = 1; // Default HDMI port
			const char fHdmiPort = 49; // '1' in ASCII
			BOOL Ret = WriteFile(hConfigFile, &fHdmiPort, 1, 0, NULL);
		}
		else
		{
			char fHdmiPort = 0;
			char* ptrHdmiPort = &fHdmiPort;
			if (ReadFile(hConfigFile, ptrHdmiPort, sizeof(fHdmiPort), NULL, NULL))
			{
				if ((fHdmiPort - 48) > 0 && (fHdmiPort - 48) < 5)
				{
					cec_config.iHDMIPort = (fHdmiPort - 48);
				}
				else
				{
					cec_config.iHDMIPort = 1; // Default HDMI port
					const char fHdmiPort = 49; // '1' in ASCII
					BOOL Ret = WriteFile(hConfigFile, &fHdmiPort, 1, 0, NULL);
				}
			}
		}
		CloseHandle(hConfigFile);
	}

	cecAdpater = LibCecInitialise(&cec_config);
	cecInit = false;
	currentMode = mode;
	if (cecAdpater)
	{
		//bool ret = cecAdpater->SetConfiguration(&cec_config);
		cecAdpater->InitVideoStandalone();
		CEC::cec_adapter_descriptor device[1];
		uint8_t iDevicesFound = cecAdpater->DetectAdapters(device, 1, NULL, true);


		if (iDevicesFound > 0)
		{
			CEC::libcec_configuration cec_configRom;
			if (cecAdpater->GetCurrentConfiguration(&cec_configRom))
			{
				if (cec_configRom.iHDMIPort != cec_config.iHDMIPort)
				{
					cec_configRom.iHDMIPort = cec_config.iHDMIPort;
					cecAdpater->SetConfiguration(&cec_config);
				}
			}
			deviceStrPort = device[0].strComName;
			cecInit = true;
		}
	}


}
MonitorHandler::~MonitorHandler()
{
	if (cecAdpater)
	{
		UnloadLibCec(cecAdpater);
		cecAdpater = 0;
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
void MonitorHandler::TogglePowerCEC(MonitorMode mode)
{
	if (cecInit)
	{
		cecAdpater->Open(deviceStrPort.c_str());
		CEC::cec_power_status pwrStatus = cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV);
		switch (currentMode)
		{
		case MonitorHandler::DESK_MODE: {
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
			break;
		}
		case MonitorHandler::BP_MODE: {
			cecAdpater->SetActiveSource(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);

			if (cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_ON ||
				cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON)
			{
				cecAdpater->SetActiveSource(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);
				if (cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_ON ||
					cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON)
				{
					cecAdpater->SetActiveSource(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);
				}
			}
			break;
		}
		}
		cecAdpater->Close();
	}
}
bool MonitorHandler::ToggleMode()
{
	switch (currentMode)
	{
		case MonitorHandler::BP_MODE:
		{
			ToggleActiveMonitors(MonitorHandler::DESK_MODE);
			currentMode = MonitorHandler::DESK_MODE;
			TogglePowerCEC(MonitorHandler::DESK_MODE);
			break;
		}
		case MonitorHandler::DESK_MODE:
		{
			if (!ToggleActiveMonitors(MonitorHandler::BP_MODE))
			{
				return false;
			}
			currentMode = MonitorHandler::BP_MODE;
			TogglePowerCEC(MonitorHandler::BP_MODE);
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
		while (isDSCEnabled())
		{
			int msgboxID = MessageBoxW(
				GetDesktopWindow(),
				(LPCWSTR)L"Display stream compression (DSC) is turned on and must be turned off to use Big Picture Mode.\nDo you want to try again?",
				(LPCWSTR)L"Display Error",
				MB_ICONERROR | MB_RETRYCANCEL | MB_DEFBUTTON2
			);

			switch (msgboxID)
			{
			case IDCANCEL:
				return false;
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
	// This is a placeholder function. Implementing actual DSC detection requires specific APIs or hardware queries.
	// For now, we'll return false to indicate DSC is not enabled.
	HRESULT hr = S_OK;
	UINT32 NumPathArrayElements = 0;
	UINT32 NumModeInfoArrayElements = 0;
	hr = GetDisplayConfigBufferSizes((QDC_ONLY_ACTIVE_PATHS), &NumPathArrayElements, &NumModeInfoArrayElements);
	std::vector<DISPLAYCONFIG_PATH_INFO> PathInfoArray2(NumPathArrayElements);
	DISPLAYCONFIG_PATH_INFO pathInfoHDMI = {};
	pathInfoHDMI.targetInfo.outputTechnology = DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER;
	std::vector<DISPLAYCONFIG_MODE_INFO> ModeInfoArray2(NumModeInfoArrayElements);
	hr = QueryDisplayConfig((QDC_ONLY_ACTIVE_PATHS), &NumPathArrayElements, &PathInfoArray2[0], &NumModeInfoArrayElements, &ModeInfoArray2[0], NULL);

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
				if (PathInfoArray2[i].targetInfo.refreshRate.Numerator > 360113)
				{
					return true;
				}
			}
		}
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