#include "AutoSettingsHandler.h"
AutoSettingsHandler::AutoSettingsHandler()
{
	HANDLE gameSettingsEvent = CreateEventW(nullptr, FALSE, FALSE, L"GameSettingsEvent");
}
AutoSettingsHandler::~AutoSettingsHandler()
{

}