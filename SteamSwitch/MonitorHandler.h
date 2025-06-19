#pragma once
class MonitorHandler
{
public:
	MonitorHandler();
	~MonitorHandler();
	enum MonitorMode {
		BP_MODE,
		DESK_MODE
	};
	int setDefaultMonitors(MonitorMode mode);
	int getDefaultMonitors(MonitorMode mode);
	void ToggleMode();

};