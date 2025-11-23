#pragma once
#include "framework.h"
constexpr int MAX_REFRESH_RATE_DSC = 360113;
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
	bool ToggleMode();
	void StartCecPowerThread(void* stmPtr);
	UINT32 getActiveMonitorCount();
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