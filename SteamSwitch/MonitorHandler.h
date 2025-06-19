#pragma once
class MonitorHandler
{
public:
	MonitorHandler();
	~MonitorHandler();

	int setDefaultMonitor(int mode);
	int getDefaultMonitor(int mode);
	void ToggleMode();

};