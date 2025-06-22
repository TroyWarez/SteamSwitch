#include "MonitorHandler.h"
#include <cecloader.h>
// It may be be possible to have two cec usb devices on the same cable

MonitorHandler::MonitorHandler(MonitorMode mode)
{
	cec_config.Clear();
	cec_config.clientVersion = CEC::LIBCEC_VERSION_CURRENT;
	cec_config.bActivateSource = 0;
	cec_config.iHDMIPort = 3; //Auto dectect HDMI port doesn't work with some devices

	cec_config.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);

	cecAdpater = LibCecInitialise(&cec_config);


	if (cecAdpater)
	{
		cecAdpater->InitVideoStandalone();
		CEC::cec_adapter_descriptor device[1];
		uint8_t iDevicesFound = cecAdpater->DetectAdapters(device, 1, NULL, true);

		cecInit = false;

		if (iDevicesFound > 0)
		{
			deviceStrPort = device[0].strComName;
			cecAdpater->Open(deviceStrPort.c_str());
			cecInit = true;
		}
	}


}
MonitorHandler::~MonitorHandler()
{
	if (cecAdpater)
	{
		cecAdpater->Close();
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
void MonitorHandler::TogglePowerCEC()
{
	if (cecInit)
	{
		CEC::cec_power_status pwrStatus = cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV);
		switch (pwrStatus)
		{
		case CEC::cec_power_status::CEC_POWER_STATUS_ON: {
			cecAdpater->SetActiveSource();
			cecAdpater->StandbyDevices(CEC::CECDEVICE_TV);
			break;
		}
		case CEC::cec_power_status::CEC_POWER_STATUS_STANDBY: {
			while (cecAdpater->GetDevicePowerStatus(CEC::CECDEVICE_TV) != CEC::cec_power_status::CEC_POWER_STATUS_ON)
			{
				cecAdpater->PowerOnDevices();
				Sleep(10);
			}
			if( !cecAdpater->IsLibCECActiveSource() )
			{
				cecAdpater->SetActiveSource();
			}
			break;
		}
		}
	}
}
void MonitorHandler::ToggleMode()
{
	switch (currentMode)
	{
		case MonitorHandler::BP_MODE:
		{
			currentMode = MonitorHandler::DESK_MODE;
			TogglePowerCEC();
			break;
		}
		case MonitorHandler::DESK_MODE:
		{
			currentMode = MonitorHandler::BP_MODE;
			TogglePowerCEC();
			break;
		}
	}
	return;
}