#include "stdafx.h"
#include "steam.h"
#include "steam_server.h"
#include <steam/steam_api.h>
#include "console.h"

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

	void shutdown()
	{
		SteamAPI_Shutdown();
		server_shutdown();
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
			/*case GameConnectedFriendChatMsg_t::k_iCallback: {
				GameConnectedFriendChatMsg_t* chat_msg = (GameConnectedFriendChatMsg_t*)callback.m_pubParam;
				std::string log_msg;
				log_msg += SteamFriends()->GetFriendPersonaName(chat_msg->m_steamIDUser);
				log_msg += ": ";
				log_msg += SteamFriends()->GetFriendMessage(chat_msg->m_steamIDUser, chat_msg->m_iMessageID);

				log_error("Received chat message from friend " + std::to_string(chat_msg->m_steamIDUser.ConvertToUint64()) + ": " + chat_msg->m_data);
			} break;*/
			}
			SteamAPI_ManualDispatch_FreeLastCallback(steam_pipe);
		}
	}

	bool is_overlay_active() {
		return _is_overlay_active;
	}

	std::string get_steam_id()
	{
		CSteamID steam_id = SteamUser()->GetSteamID();
		return std::to_string(steam_id.ConvertToUint64());
	}

	std::vector<std::string> get_friends_persona_names()
	{
		std::vector<std::string> names;
		int friends_count = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
		for (int i = 0; i < friends_count; i++) {
			CSteamID friend_id = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);
			names.push_back(SteamFriends()->GetFriendPersonaName(friend_id));
		}
		return names;
	}
}