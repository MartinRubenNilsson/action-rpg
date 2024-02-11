#include "console.h"
#include "window.h"
#include "audio.h"
#include "map.h"
#include "ui.h"
#include "ecs_player.h"
#include "ecs_tiled.h"
#include "ecs_camera.h"
#include "shaders.h"

namespace console
{
	void _kill_player() {
		ecs::kill_player(ecs::find_entity_by_class("player"));
	}

	void _do_execute(const std::string& command_line)
	{
		std::istringstream iss(command_line);
		std::string cmd;
		if (!(iss >> cmd)) return;

		bool help = (cmd == "help");
		if (help) {
			if (!(iss >> cmd)) return;
		}

		bool error = false;
		bool cmd_found = false;
		std::string help_msg;
		std::string error_msg;

#define HELP(desc) \
		if (help) { help_msg += desc; }

#define ARG(type, name, desc) \
		type name{}; \
		if (help) { help_msg += "\n" ## #type ## " " ## #name ## ": " ## desc; } \
		else if (!(iss >> name)) { error = true; error_msg += "Expected: " ## #type ## " " ## #name; }

#define ARGLINE(name, desc) \
		std::string name{}; \
		if (help) { help_msg += "\nstd::string " ## #name ## ": " ## desc; } \
		else { iss.ignore(64, ' '); std::getline(iss, name); }

#define EXEC(expr) \
		cmd_found = true; if (!help && !error) { expr; }

		// CONSOLE

		if (cmd == "clear") {
			HELP("Clear the console");
			EXEC(console::clear());
		}
		if (cmd == "sleep") {
			HELP("Sleep for a number of seconds");
			ARG(float, seconds, "The number of seconds to sleep");
			EXEC(console::sleep(seconds));
		}
		if (cmd == "log") {
			HELP("Log a message to the console");
			ARGLINE(message, "The message to log");
			EXEC(console::log(message));
		}
		if (cmd == "log_error") {
			HELP("Log an error message to the console");
			ARGLINE(message, "The message to log");
			EXEC(console::log_error(message));
		}
		if (cmd == "execute") {
			HELP("Execute a console command");
			ARGLINE(command_line, "The command to execute");
			EXEC(console::execute(command_line));
		}
		if (cmd == "execute_script") {
			HELP("Execute a console script");
			ARG(std::string, script_name, "The name of the script");
			EXEC(console::execute_script(script_name));
		}
		if (cmd == "bind") {
			HELP("Bind a key to a console command");
			ARG(std::string, key_string, "The key to bind");
			ARGLINE(command_line, "The command to execute");
			EXEC(console::bind(key_string, command_line));
		}
		if (cmd == "unbind") {
			HELP("Unbind a key");
			ARG(std::string, key_string, "The key to unbind");
			EXEC(console::unbind(key_string));
		}

		// SHADERS

		if (cmd == "reload_shaders") {
			HELP("Reload all shaders");
			EXEC(shaders::reload_assets());
		}

		// AUDIO

		if (cmd == "audio_play") {
			HELP("Play an audio event");
			ARG(std::string, event_name, "The name of the event");
			EXEC(audio::play(event_name));
		}

		// MAPS

		if (cmd == "map_open") {
			HELP("Open a map");
			ARG(std::string, map_name, "The name of the map");
			EXEC(map::open(map_name));
		}
		if (cmd == "map_close") {
			HELP("Close the current map");
			EXEC(map::close());
		}
		if (cmd == "map_reset") {
			HELP("Reset the current map");
			EXEC(map::reset());
		}

		// UI

		if (cmd == "ui_show") {
			HELP("Show an RML document");
			ARG(std::string, name, "The name of the document");
			EXEC(ui::show_document(name));
		}

		// CAMERA

		if (cmd == "add_camera_trauma") {
			HELP("Add trauma to the camera to make it shake");
			ARG(float, trauma, "The amount of trauma to add");
			EXEC(ecs::add_camera_trauma(trauma));
		}

		// MISC

		if (cmd == "kill_player") {
			HELP("Kill the player");
			EXEC(_kill_player());
		}

		if (error)
			log_error(error_msg);
		else if (!cmd_found)
			log_error("Unknown command: " + cmd);
		else if (help)
			log(help_msg);
	}
}
