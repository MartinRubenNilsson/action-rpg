#include "stdafx.h"
#include "steam.h"
#include <steam/steam_api.h>

#pragma comment(lib, "steam_api64")

namespace steam
{
	const uint32_t _APP_ID = 0; // Invalid ID for now
	bool _is_overlay_active = false;

	bool restart_app_if_necessary() {
		return SteamAPI_RestartAppIfNecessary(_APP_ID);
	}

	bool is_steam_running() {
		return SteamAPI_IsSteamRunning();
	}

	bool initialize()
	{
		if (!SteamAPI_Init())
			return false;
		SteamAPI_ManualDispatch_Init();
		return true;
	}

	void shutdown() {
		SteamAPI_Shutdown();
	}

	void run_message_loop()
	{
		HSteamPipe steam_pipe = SteamAPI_GetHSteamPipe();
		SteamAPI_ManualDispatch_RunFrame(steam_pipe);
		CallbackMsg_t callback{};
		while (SteamAPI_ManualDispatch_GetNextCallback(steam_pipe, &callback)) {
			switch (callback.m_iCallback) {
			//case SteamAPICallCompleted_t::k_iCallback: {
			//	SteamAPICallCompleted_t* call_completed = (SteamAPICallCompleted_t*)callback.m_pubParam;
			//	void* call_result = malloc(call_completed->m_cubParam);
			//	bool failed = true;
			//	if (SteamAPI_ManualDispatch_GetAPICallResult(
			//		steam_pipe,
			//		call_completed->m_hAsyncCall,
			//		call_result,
			//		call_completed->m_cubParam,
			//		call_completed->m_iCallback,
			//		&failed))
			//	{
			//		// Dispatch the call result to the registered handler(s) for the
			//		// call identified by pCallCompleted->m_hAsyncCall
			//	}
			//	free(call_result);
			//} break;
			case GameOverlayActivated_t::k_iCallback: {
				GameOverlayActivated_t* overlay_activated = (GameOverlayActivated_t*)callback.m_pubParam;
				_is_overlay_active = overlay_activated->m_bActive;
			} break;
			}
			SteamAPI_ManualDispatch_FreeLastCallback(steam_pipe);
		}
	}

	bool is_overlay_active() {
		return _is_overlay_active;
	}
}