#include "MonitorHandler.h"
#include "AudioHandler.h"
#include "steam_api.h"
int main()
{
	MonitorHandler* monHandler = new MonitorHandler(MonitorHandler::DESK_MODE);
	monHandler->ToggleMode();
	//auto a = monHandler->getMonitorMode();
//	if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid)) // Replace with your App ID
//	{
//		return 1;
//	}

	//if (!SteamAPI_Init())
//	{
//		printf("Fatal Error - Steam must be running to play this game (SteamAPI_Init() failed).\n");
	//	return 1;
	//}
	//unsigned int batteryVal = SteamUtils()->GetCurrentBatteryPower();
	// 
	delete monHandler;
	AudioHandler* audioHandler = new AudioHandler();
	return 0;
}