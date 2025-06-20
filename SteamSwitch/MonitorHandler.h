#pragma once
class MonitorHandler
{
private:
	enum MonitorMode {
		BP_MODE,
		DESK_MODE,
		DEFAULT_MODE
	};
	MonitorMode currentMode;
public:
	MonitorHandler();
	~MonitorHandler();
	int setDefaultMonitors(MonitorMode mode);
	MonitorMode getDefaultMonitors();
	void ToggleMode();

};