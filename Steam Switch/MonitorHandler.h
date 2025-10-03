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
	void StandByAllDevicesCEC();
	void StartCecPowerThread(void* stmPtr);
	int getActiveMonitorCount();
	HANDLE hCECThread;
	HANDLE hCECPowerOffEvent;
	HANDLE hCECPowerOnEvent;
	HANDLE hCECPowerOffFinishedEvent;
	HANDLE hShutdownEvent;
	bool icueInstalled;
private:
	MonitorMode currentMode;
	bool ToggleActiveMonitors(MonitorMode mode);
	bool isDSCEnabled();
};