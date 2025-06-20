#include "MonitorHandler.h"
#include <cec.h>
// It may be be possible to have two cec usb devices on the same cable
MonitorHandler::MonitorHandler(MonitorMode mode)
{
	currentMode = mode;
}
MonitorHandler::~MonitorHandler()
{
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
			break;
		}
		case MonitorHandler::DESK_MODE:
		{
			break;
		}
	}
	return;
}