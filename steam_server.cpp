#include "stdafx.h"
#include "steam_server.h"
#include <steam/steam_gameserver.h>
#include "console.h"

// After initializing a game server you have access to the two game server
// exclusive interfaces ISteamGameServer and ISteamGameServerStats.
// 
// You can also access the following regular interfaces from the game server:
// ISteamClient which you can access via the global interface: SteamGameServerClient()
// ISteamUtils which you can access via the global interface: SteamGameServerUtils()
// ISteamNetworking which you can access via the global interface: SteamGameServerNetworking()
// ISteamHTTP which you can access via the global interface: SteamGameServerHTTP()
// ISteamUGC which you can access via the global interface: SteamGameServerUGC()
// ISteamApps which you can access via the global interface: SteamGameServerApps()

namespace steam
{
	const uint32_t _GAME_PORT = 27015;
	const uint32_t _QUERY_PORT = 27016;
	const char _SERVER_VERSION_STRING[] = "0.0.0.0";
	HSteamListenSocket _listen_socket = k_HSteamListenSocket_Invalid;

	bool server_initialize()
	{
		if (SteamGameServer()) {
			console::log_error("Failed to initialize Steam server: already initialized.");
			return false;
		}
		if (!SteamGameServer_Init(0, _GAME_PORT, _QUERY_PORT, eServerModeNoAuthentication, _SERVER_VERSION_STRING)) {
			console::log_error("Failed to initialize Steam server: SteamGameServer_Init failed.");
			return false;
		}
		SteamGameServer()->SetProduct("Action RPG");
		SteamGameServer()->SetGameDescription("Action RPG");
		SteamGameServer()->LogOnAnonymous();
		//SteamNetworkingUtils()->InitRelayNetworkAccess();
		SteamNetworkingIPAddr server_addr{};
		server_addr.Clear();
		server_addr.m_port = _GAME_PORT;
		_listen_socket = SteamGameServerNetworkingSockets()->CreateListenSocketIP(server_addr, 0, nullptr);
		return true;
	}

	void server_shutdown()
	{
		if (!SteamGameServer()) return;
		SteamGameServerNetworkingSockets()->CloseListenSocket(_listen_socket);
		SteamGameServer()->LogOff();
		SteamGameServer_Shutdown();
	}

	/*void server_log_on_anonymous()
	{
		if (!SteamGameServer()) return;
		SteamGameServer()->LogOnAnonymous();
	}

	void server_log_off()
	{
		if (!SteamGameServer()) return;
		SteamGameServer()->LogOff();
	}

	bool server_logged_on()
	{
		if (!SteamGameServer()) return false;
		return SteamGameServer()->BLoggedOn();
	}*/

	std::string server_get_public_ip()
	{
		if (!SteamGameServer()) return std::string();
		SteamIPAddress_t ip = SteamGameServer()->GetPublicIP();
		SteamNetworkingIPAddr ip_addr;
		ip_addr.Clear();
		if (ip.m_eType == k_ESteamIPTypeIPv4) {
			ip_addr.SetIPv4(ip.m_unIPv4, 0);
		} else {
			ip_addr.SetIPv6(ip.m_rgubIPv6, 0);
		}
		char ip_str[SteamNetworkingIPAddr::k_cchMaxString];
		ip_addr.ToString(ip_str, SteamNetworkingIPAddr::k_cchMaxString, false);
		return ip_str;
	}
}