#include "MonitorHandler.h"
#include <cec.h>
// It may be be possible to have two cec usb devices on the same cable
MonitorHandler::MonitorHandler()
{
	currentMode = DEFAULT_MODE;
}
MonitorHandler::~MonitorHandler()
{
}
int MonitorHandler::setDefaultMonitors(MonitorMode mode)
{
	currentMode = mode;
	return 0;
}
MonitorHandler::MonitorMode MonitorHandler::getDefaultMonitors()
{
	return currentMode;
}
void MonitorHandler::ToggleMode()
{
	return;
}