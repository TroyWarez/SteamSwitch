#pragma once
#include "framework.h"
constexpr LPCWSTR settingsSearchFolderPathEnv = L"%PROGRAMFILES%\\Steam Switch\\AutoSettings\\*";
class AutoSettingsHandler
{
	public:
	AutoSettingsHandler();
	~AutoSettingsHandler();

	std::wstring settingsSearchPath;

	std::vector<std::wstring> autoSettingsPaths;
};

