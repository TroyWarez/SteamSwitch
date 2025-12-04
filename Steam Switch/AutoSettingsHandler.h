#pragma once
#include "framework.h"
#include "MonitorHandler.h"
constexpr LPCWSTR BPsettingsSearchFolderPathEnv = L"%PROGRAMFILES%\\Steam Switch\\AutoSettings\\BP\\*";
constexpr LPCWSTR DESKsettingsSearchFolderPathEnv = L"%PROGRAMFILES%\\Steam Switch\\AutoSettings\\DESK\\*";
class AutoSettingsHandler
{
private:
	DWORD ApplySettingsFromFolder(BOOL isBigPictureMode);
public:
	AutoSettingsHandler();
	~AutoSettingsHandler();

	std::wstring BPsettingsSearchPath;
	std::wstring DESKsettingsSearchPath;

	std::vector<std::wstring> BPautoSettingsPaths;
	std::vector<std::wstring> DESKautoSettingsPaths;

	std::vector<std::wstring> BPFolderSettingsPaths;
	std::vector<std::wstring> DESKFolderSettingsPaths;

	MonitorHandler* monHandler;
	DWORD SetAllBPModeSettings();
	DWORD SetAllDESKModeSettings();
};

