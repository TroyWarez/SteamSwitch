#include "steam_api.h"
#include "SteamHandler.h"
int main()
{
	SteamHandler* steamHandler = new SteamHandler();
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
	//delete monHandler;
	// 	delete audioHandler;
	//AudioHandler* audioHandler = new AudioHandler();
	//audioHandler->setDefaultAudioDevice(0);

	delete steamHandler;
	return 0;
}