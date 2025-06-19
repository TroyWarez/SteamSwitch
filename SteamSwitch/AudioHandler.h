#pragma once
class AudioHandler
{
public:
	AudioHandler();
	~AudioHandler();
	int setDefaultAudioDevice(int device);
	int getDefaultAudioDevice(int device);
	void ToggleMode();
};

