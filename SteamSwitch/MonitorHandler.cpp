#include "MonitorHandler.h"
#include <cecloader.h>
// It may be be possible to have two cec usb devices on the same cable

MonitorHandler::MonitorHandler(MonitorMode mode)
{
	cec_config.Clear();
	cec_config.clientVersion = CEC::LIBCEC_VERSION_CURRENT;
	cec_config.bActivateSource = 0;

	cec_config.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);

	cecAdpater = LibCecInitialise(&cec_config);

	currentMode = mode;
}
MonitorHandler::~MonitorHandler()
{
	if (cecAdpater)
	{
		UnloadLibCec(cecAdpater);
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
void MonitorHandler::ToggleMode()
{
	switch (currentMode)
	{
		case MonitorHandler::BP_MODE:
		{
			currentMode = MonitorHandler::DESK_MODE;
			break;
		}
		case MonitorHandler::DESK_MODE:
		{
			currentMode = MonitorHandler::BP_MODE;
			break;
		}
	}
	return;
}