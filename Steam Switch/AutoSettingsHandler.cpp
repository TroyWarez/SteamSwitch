#include "AutoSettingsHandler.h"
AutoSettingsHandler::AutoSettingsHandler()
{
	//HANDLE gameSettingsEvent = CreateEventW(nullptr, FALSE, FALSE, L"GameSettingsEvent");
	std::array<WCHAR, MAX_PATH>  userProfile = { L'\0' };
	ExpandEnvironmentStringsW(settingsSearchFolderPathEnv, userProfile.data(), MAX_PATH);
	settingsSearchPath = userProfile.data();
}
AutoSettingsHandler::~AutoSettingsHandler()
{

}