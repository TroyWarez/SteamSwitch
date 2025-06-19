#pragma once
class MonitorHandler
{
private:

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