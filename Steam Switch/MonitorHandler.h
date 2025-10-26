#pragma once
#include "framework.h"
#define MAX_REFRESH_RATE_DSC 360113
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
	void StartCecPowerThread(void* stmPtr);
	int getActiveMonitorCount();
	bool isSingleDisplayHDMI();
	bool isDSCEnabled();
	HANDLE hCECThread;
	HANDLE hCECPowerOffEvent;
	HANDLE hCECPowerOnEvent;
	HANDLE hCECPowerOffFinishedEvent;
	HANDLE hShutdownEvent;
	bool icueInstalled;
private:
	MonitorMode currentMode;
	bool ToggleActiveMonitors(MonitorMode mode);
};