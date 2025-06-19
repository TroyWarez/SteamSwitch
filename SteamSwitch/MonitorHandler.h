#pragma once
class MonitorHandler
{
	MonitorHandler();
	~MonitorHandler();

	int setDefaultMonitor(int mode);
	int getDefaultMonitor(int mode);
	void ToggleMode();

};