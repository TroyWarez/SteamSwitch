#pragma once
#include "framework.h"
constexpr LPCWSTR BPsettingsSearchFolderPathEnv = L"%PROGRAMFILES%\\Steam Switch\\AutoSettings\\BP\\*";
constexpr LPCWSTR DESKsettingsSearchFolderPathEnv = L"%PROGRAMFILES%\\Steam Switch\\AutoSettings\\DESK\\*";
class AutoSettingsHandler
{
private:
	void ApplySettingsFromFolder(BOOL isBigPictureMode);
public:
	AutoSettingsHandler();
	~AutoSettingsHandler();

	std::wstring BPsettingsSearchPath;
	std::wstring DESKsettingsSearchPath;

	std::vector<std::wstring> BPautoSettingsPaths;
	std::vector<std::wstring> DESKautoSettingsPaths;
	void SetAllBPModeSettings();
	void SetAllDESKModeSettings();
};

