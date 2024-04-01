#include "stdafx.h"
#include "console_commands.h"
#include "console.h"
#include "steam.h"
#include "steam_server.h"
#include "steam_client.h"

namespace console
{
	void register_commands_steam(std::vector<Command>& commands)
	{
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "steam_id";
			cmd.desc = "Prints the Steam ID of the current user";
			cmd.callback = [](const Params&) {
				log(steam::get_steam_id());
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "steam_friends";
			cmd.desc = "Prints the names of the friends of the current user";
			cmd.callback = [](const Params&) {
				auto names = steam::get_friends_persona_names();
				for (const std::string& name : names)
					log(name);
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "server_init";
			cmd.desc = "Initializes the server";
			cmd.callback = [](const Params&) {
				steam::server_initialize();
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "server_shutdown";
			cmd.desc = "Shuts down the server";
			cmd.callback = [](const Params&) {
				steam::server_shutdown();
			};
		}
		/*{
			Command& cmd = commands.emplace_back();
			cmd.name = "server_log_on_anonymous";
			cmd.desc = "Logs on the server anonymously";
			cmd.callback = [](const Params&) {
				steam::server_log_on_anonymous();
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "server_log_off";
			cmd.desc = "Logs off the server";
			cmd.callback = [](const Params&) {
				steam::server_log_off();
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "server_logged_on";
			cmd.desc = "Prints whether the server is logged on";
			cmd.callback = [](const Params&) {
				log(steam::server_logged_on() ? "true" : "false");
			};
		}*/
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "server_ip";
			cmd.desc = "Prints the public IP of the server";
			cmd.callback = [](const Params&) {
				log(steam::server_get_public_ip());
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "client_connect";
			cmd.desc = "Connects the client to a server";
			cmd.params[0] = { "", "IP", "The IP address of the server" };
			cmd.callback = [](const Params& params) {
				steam::connect_client_to_server_by_ip(std::get<std::string>(params[0].arg));
			};
		}
	}
}