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
	bool ToggleMode();
	void TogglePowerCEC(MonitorMode mode);
	int getActiveMonitorCount();
private:
	MonitorMode currentMode;
	CEC::libcec_configuration cec_config;
	CEC::ICECAdapter* cecAdpater;
	std::string deviceStrPort;
	bool cecInit;
	bool ToggleActiveMonitors(MonitorMode mode);
	bool isDSCEnabled();
};