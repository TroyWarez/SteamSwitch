#pragma once
#include <cec.h>
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
	void ToggleMode();
private:
	MonitorMode currentMode;
	CEC::libcec_configuration cec_config;
	CEC::ICECAdapter* cecAdpater;
	std::string deviceStrPort;
	bool cecInit;
	void TogglePowerCEC();
	void ToggleActiveMonitors(MonitorMode mode);
};