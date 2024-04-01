#include "stdafx.h"
#include "steam_client.h"
#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include "console.h"

namespace steam
{
	const uint32_t _GAME_PORT = 27015;
	HSteamNetConnection _client_to_server_connection = k_HSteamNetConnection_Invalid;

	void connect_client_to_server_by_ip(const std::string& server_ip)
	{
		if (!SteamNetworkingSockets()) return;
		SteamNetworkingIPAddr server_addr;
		server_addr.Clear();
		if (!server_addr.ParseString(server_ip.c_str())) {
			console::log_error("Failed to parse server IP address: " + server_ip);
			return;
		}
		server_addr.m_port = _GAME_PORT;
		_client_to_server_connection = SteamNetworkingSockets()->ConnectByIPAddress(server_addr, 0, nullptr);
	}
}