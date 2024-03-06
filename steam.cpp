#include "stdafx.h"
#include "steam.h"
#include <steam/steam_api.h>

#pragma comment(lib, "steam_api64")

namespace steam
{
	const uint32_t _APP_ID = 0; // Invalid ID for now

	bool restart_app_if_necessary() {
		return SteamAPI_RestartAppIfNecessary(_APP_ID);
	}

	void initialize() {
		SteamAPI_Init();
	}

	void shutdown() {
		SteamAPI_Shutdown();
	}
}