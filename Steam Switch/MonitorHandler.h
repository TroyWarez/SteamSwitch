#pragma once
#include "framework.h"
class MonitorHandler
{
public:
	enum MonitorMode {
		BP_MODE,
		DESK_MODE
	};
	MonitorHandler(MonitorMode mode);
	~MonitorHandler();
	void setMonitorMode(MonitorMode mode);
	MonitorMode getMonitorMode();
	bool ToggleMode(bool isIcueInstalled);
	void TogglePowerCEC(MonitorMode mode);
	int getActiveMonitorCount();
	HANDLE hCECThread;
	HANDLE hCECPowerOffEvent;
	HANDLE hCECPowerOnEvent;
	HANDLE hShutdownEvent;
	HANDLE hMonitorThread;
	bool icueInstalled;
private:
	MonitorMode currentMode;
	bool ToggleActiveMonitors(MonitorMode mode);
	bool isDSCEnabled();
};