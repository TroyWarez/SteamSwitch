#pragma once
class AudioHandler
{
private: 
	void InitDefaultAudioDevice();
public:
	AudioHandler();
	~AudioHandler();
	int setDefaultAudioDevice(int device);
	int getDefaultAudioDevice(int device);
	void ToggleMode();
};

