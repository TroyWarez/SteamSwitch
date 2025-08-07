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
private:
	HANDLE hCECThread;
	MonitorMode currentMode;
	CEC::libcec_configuration cec_config;
	bool cecInit;
	bool icueInstalled;
	bool ToggleActiveMonitors(MonitorMode mode);
	bool isDSCEnabled();
};