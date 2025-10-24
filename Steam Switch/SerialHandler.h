#pragma once
struct GIPSerialData {
	unsigned short pwrStatus;
	unsigned short controllerCount;
};
class SerialHandler
{
public:
	std::wstring devicePath;
	std::wstring comPath;
	std::wstring devicePort;
	HANDLE hSerial;
	void ScanForSerialDevices();
	SerialHandler();
	~SerialHandler();
};

