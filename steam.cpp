#include "stdafx.h"
#include "steam.h"
#include <steam/steam_api.h>

#pragma comment(lib, "steam_api64")

namespace steam
{
	bool initialize() {
		return SteamAPI_Init();
	}

	void shutdown() {
		SteamAPI_Shutdown();
	}
}