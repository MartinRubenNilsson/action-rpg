#include "stdafx.h"
#include "console_commands.h"
#include "console.h"
#include "window.h"
#include "audio.h"
#include "map.h"
#include "ui.h"
#include "shaders.h"
#include "ecs_player.h"
#include "ecs_common.h"
#include "ecs_camera.h"
#include "ecs_vfx.h"

namespace console
{
	void register_commands_misc(std::vector<Command>& commands)
	{
		// CONSOLE
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "help";
			cmd.desc = "Shows help for a command";
			cmd.params[0] = { "", "command", "The command to show help for" };
			cmd.callback = [](const Params& params) {
				auto it = find_command(std::get<std::string>(params[0].arg));
				if (it != commands_end())
					log(format_help_message(*it));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "clear";
			cmd.desc = "Clears the console";
			cmd.callback = [](const Params&) { clear(); };
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "sleep";
			cmd.desc = "Defers incoming commands, executing them later";
			cmd.params[0] = { 0.f, "seconds", "The number of seconds to sleep" };
			cmd.callback = [](const Params& params) {
				sleep(std::get<float>(params[0].arg));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "log";
			cmd.desc = "Logs a message to the console";
			cmd.params[0] = { "", "message", "The message to log" };
			cmd.callback = [](const Params& params) {
				log(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "log_error";
			cmd.desc = "Logs an error message to the console";
			cmd.params[0] = { "", "message", "The message to log" };
			cmd.callback = [](const Params& params) {
				log_error(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "execute";
			cmd.desc = "Executes a console command";
			cmd.params[0] = { "", "command_line", "The command to execute" };
			cmd.callback = [](const Params& params) {
				execute(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "execute_script";
			cmd.desc = "Executes a console script";
			cmd.params[0] = { "", "script_name", "The name of the script" };
			cmd.callback = [](const Params& params) {
				execute_script(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "bind";
			cmd.desc = "Binds a console command to a key";
			cmd.params[0] = { "", "key", "The key to bind" };
			cmd.params[1] = { "", "command_line", "The command to execute" };
			cmd.callback = [](const Params& params) {
				bind(std::get<std::string>(params[0].arg), std::get<std::string>(params[1].arg));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "unbind";
			cmd.desc = "Unbinds a key";
			cmd.params[0] = { "", "key", "The key to unbind" };
			cmd.callback = [](const Params& params) {
				unbind(std::get<std::string>(params[0].arg));
			};
		}

		// SHADERS
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "reload_shaders";
			cmd.desc = "Reloads all shaders";
			cmd.callback = [](const Params& params) { shaders::reload_assets(); };
		}

		// AUDIO
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "audio_play";
			cmd.desc = "Plays an audio event";
			cmd.params[0] = { "", "event_path", "The full path of the event" };
			cmd.callback = [](const Params& params) {
				audio::play(std::get<std::string>(params[0].arg));
			};
		}

		// MAP
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "map_open";
			cmd.desc = "Opens a map";
			cmd.params[0] = { "", "name", "The name of the map" };
			cmd.callback = [](const Params& params) {
				map::open(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "map_close";
			cmd.desc = "Closes the current map";
			cmd.callback = [](const Params& params) { map::close(); };
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "map_reset";
			cmd.desc = "Resets the current map";
			cmd.callback = [](const Params& params) { map::reset(); };
		}

		// UI
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "ui_show";
			cmd.desc = "Shows an RML document";
			cmd.params[0] = { "", "name", "The name of the document" };
			cmd.callback = [](const Params& params) {
				ui::show_document(std::get<std::string>(params[0].arg));
			};
		}

		// GAME
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "destroy";
			cmd.desc = "Destroy an entity";
			cmd.params[0] = { 0, "entity", "The ID of the entity to destroy" };
			cmd.callback = [](const Params& params) {
				ecs::destroy_at_end_of_frame((entt::entity)std::get<int>(params[0].arg));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "clone";
			cmd.desc = "Clone an entity";
			cmd.params[0] = { 0, "entity", "The ID of the entity to clone" };
			cmd.callback = [](const Params& params) {
				ecs::deep_copy((entt::entity)std::get<int>(params[0].arg));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "kill_player";
			cmd.desc = "Kills the player";
			cmd.callback = [](const Params&) {
				ecs::kill_player(ecs::find_entity_by_class("player"));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "add_camera_shake";
			cmd.desc = "Adds trauma to the active camera to make it shake";
			cmd.params[0] = { 0.f, "trauma", "The amount of trauma to add" };
			cmd.callback = [](const Params& params) {
				ecs::add_trauma_to_active_camera(std::get<float>(params[0].arg));
			};
		}
		{
			Command& cmd = commands.emplace_back();
			cmd.name = "create_vfx";
			cmd.desc = "Spawns a VFX in the game world";
			cmd.params[0] = { "", "type", "The type of VFX" };
			cmd.params[1] = { sf::Vector2f(), "position", "The position to spawn the VFX at" };
			cmd.callback = [](const Params& params) {
				const std::string& type_str = std::get<std::string>(params[0].arg);
				const sf::Vector2f& position = std::get<sf::Vector2f>(params[1].arg);
				auto type = magic_enum::enum_cast<ecs::VfxType>(type_str, magic_enum::case_insensitive);
				if (type.has_value()) {
					create_vfx(type.value(), position);
				} else {
					log_error("Unknown VFX type: " + type_str);
				}
			};
		}
	}
}