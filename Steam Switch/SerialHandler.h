#pragma once
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

