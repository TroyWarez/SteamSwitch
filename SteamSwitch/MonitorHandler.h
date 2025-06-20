#pragma once
class MonitorHandler
{
private:
	enum MonitorMode {
		BP_MODE,
		DESK_MODE
	};
	MonitorMode currentMode;
public:
	MonitorHandler(MonitorMode mode);
	~MonitorHandler();
	void setMonitorMode(MonitorMode mode);
	MonitorMode getMonitorMode();
	void ToggleMode();

};