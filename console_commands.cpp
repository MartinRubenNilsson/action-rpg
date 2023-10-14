#include "console.h"
#include <CLI/CLI.hpp>
#include "window.h"
#include "audio.h"
#include "map.h"
#include "data.h"
#include "ui.h"
#include "ui_textbox.h"
#include "ecs_console_commands.h"

namespace console
{
	void _initialize_commands(CLI::App& app)
	{
		// CONSOLE
		{
			app.add_subcommand("clear", "Clear the console")
				->callback(console::clear);
			app.add_subcommand("sleep", "Sleep for a number of seconds")
				->add_option_function<float>("seconds", console::sleep, "The number of seconds")
				->required();
			app.add_subcommand("log", "Log a message to the console")
				->add_option_function<std::string>("message", console::log, "The message to log")
				->required();
			app.add_subcommand("script", "Execute a console script")
				->add_option_function<std::string>("name", console::execute_script, "The name of the script")
				->required();
			{
				static std::string key_string;
				static std::string command_line;
				auto bind_cmd = app.add_subcommand("bind", "Bind a key to a console command");
				bind_cmd->add_option("key", key_string, "The key to bind")
					->required();
				bind_cmd->add_option("command", command_line, "The command to execute")
					->required();
				bind_cmd->callback([]() { console::bind(key_string, command_line); });
			}
			{
				static std::string key_string;
				auto bind_cmd = app.add_subcommand("unbind", "Unbind a key");
				bind_cmd->add_option("key", key_string, "The key to unbind")
					->required();
				bind_cmd->callback([]() { console::unbind(key_string); });
			}
		}

		// WINDOW
		{
			auto cmd = app.add_subcommand("window", "Manage the window");
			cmd->add_subcommand("close", "Close the window")
				->callback(window::close);
			cmd->add_subcommand("title", "Set the window title")
				->add_option_function<std::string>("title", window::set_title, "The new title of the window")
				->required();
			cmd->add_subcommand("scale", "Set the window scale")
				->add_option_function<uint32_t>("title", window::set_scale, "The new scale of the window")
				->required();
		}

		// AUDIO
		{
			auto cmd = app.add_subcommand("audio", "Manage audio");
			cmd->add_subcommand("play", "Play an audio event")
				->add_option_function<std::string>("path", audio::play, "The path of the sound event")
				->required();
			cmd->add_subcommand("stop_all", "Stop all audio events")
				->callback(audio::stop_all);
		}

		// MAP
		{
			auto cmd = app.add_subcommand("map", "Manage maps");
			cmd->add_subcommand("name", "Print the name of the current map")
				->callback([]() { log(map::get_name()); });
			{
				auto cmd_open = cmd->add_subcommand("open", "Open a map");
				static std::string map_name;
				static bool force_open = false;
				cmd_open->add_option("name", map_name, "The name of the map")->required();
				cmd_open->add_flag("-f,--force", force_open, "Force the map to open");
				cmd_open->callback([]() { map::open(map_name, force_open); });
			}
			cmd->add_subcommand("reopen", "Reopen the current map")
				->callback(map::reopen);
			cmd->add_subcommand("close", "Close the current map")
				->callback(map::close);
			cmd->add_subcommand("reload", "Reload all textures used by the current map")
				->callback(map::reload_textures);
			cmd->add_subcommand("spawnpoint", "Set the player spawnpoint entity")
				->add_option_function<std::string>("name", map::set_spawnpoint,
					"The name of the spawnpoint entity")
				->required();
		}

		// DATA
		{
			auto cmd = app.add_subcommand("data", "Manage data");
			cmd->add_subcommand("clear", "Clear all data")
				->callback(data::clear);
			cmd->add_subcommand("load", "Load data from a JSON file")
				->add_option_function<std::string>("name", data::load, "The name of the file")
				->required();
			cmd->add_subcommand("save", "Save data to a JSON file")
				->add_option_function<std::string>("name", data::save, "The name of the file")
				->required();
			cmd->add_subcommand("dump", "Dump all data as a string to the console")
				->callback([]() { log(data::dump()); });
			{
				auto cmd_set = cmd->add_subcommand("set_bool", "Set a data bool");
				static std::string key;
				static bool value;
				cmd_set->add_option("key", key, "The key of the data")->required();
				cmd_set->add_option("value", value, "The value of the data")->required();
				cmd_set->callback([]() { data::set_bool(key, value); });
			}
			{
				auto cmd_get = cmd->add_subcommand("get_bool", "Get a data bool");
				static std::string key;
				cmd_get->add_option("key", key, "The key of the data")->required();
				cmd_get->callback([]() {
					bool value;
					if (data::get_bool(key, value)) log(std::to_string(value));
					else log_error("Failed to get data bool: " + key);
				});
			}
			{
				auto cmd_set = cmd->add_subcommand("set_int", "Set a data int");
				static std::string key;
				static int value;
				cmd_set->add_option("key", key, "The key of the data")->required();
				cmd_set->add_option("value", value, "The value of the data")->required();
				cmd_set->callback([]() { data::set_int(key, value); });
			}
			{
				auto cmd_get = cmd->add_subcommand("get_int", "Get a data int");
				static std::string key;
				cmd_get->add_option("key", key, "The key of the data")->required();
				cmd_get->callback([]() {
					int value;
					if (data::get_int(key, value)) log(std::to_string(value));
					else log_error("Failed to get data int: " + key);
				});
			}
			{
				auto cmd_set = cmd->add_subcommand("set_float", "Set a data float");
				static std::string key;
				static float value;
				cmd_set->add_option("key", key, "The key of the data")->required();
				cmd_set->add_option("value", value, "The value of the data")->required();
				cmd_set->callback([]() { data::set_float(key, value); });
			}
			{
				auto cmd_get = cmd->add_subcommand("get_float", "Get a data float");
				static std::string key;
				cmd_get->add_option("key", key, "The key of the data")->required();
				cmd_get->callback([]() {
					float value;
					if (data::get_float(key, value)) log(std::to_string(value));
					else log_error("Failed to get data float: " + key);
				});
			}
			{
				auto cmd_set = cmd->add_subcommand("set_string", "Set a data string");
				static std::string key;
				static std::string value;
				cmd_set->add_option("key", key, "The key of the data")->required();
				cmd_set->add_option("value", value, "The value of the data")->required();
				cmd_set->callback([]() { data::set_string(key, value); });
			}
			{
				auto cmd_get = cmd->add_subcommand("get_string", "Get a data string");
				static std::string key;
				cmd_get->add_option("key", key, "The key of the data")->required();
				cmd_get->callback([]() {
					std::string value;
					if (data::get_string(key, value)) log(value);
					else log_error("Failed to get data string: " + key);
				});
			}
		}

		// UI
		{
			auto cmd = app.add_subcommand("ui", "Manage UI");
			cmd->add_subcommand("reload", "Reload all style sheets")
				->callback(ui::reload_style_sheets);
			cmd->add_subcommand("show", "Show a document")
				->add_option_function<std::string>("name", ui::show, "The name of the document")
				->required();
			cmd->add_subcommand("hide", "Hide a document")
				->add_option_function<std::string>("name", ui::hide, "The name of the document")
				->required();
			
		}

		// UI - TEXTBOX
		{
			auto cmd = app.add_subcommand("textbox", "Manage the textbox");
			cmd->add_subcommand("add", "Add textbox entries to the end of the queue")
				->add_option_function<std::string>("name",
					ui::add_textbox_entries, "The name of the entries")
				->required();
			cmd->add_subcommand("set", "Set the queue to the given textbox entries")
				->add_option_function<std::string>("name",
					ui::set_textbox_entries, "The name of the entries")
				->required();
		}

		// ECS
		{
			auto ecs_cmd = app.add_subcommand("ecs", "Manage the ECS");
			{
				auto cmd = ecs_cmd->add_subcommand("set_camera_priority", "Set the priority of a camera");
				static std::string entity_name;
				static float priority;
				cmd->add_option("entity_name", entity_name, "The name of the camera entity")
					->required();
				cmd->add_option("priority", priority, "The priority of the camera")
					->required();
				cmd->callback([]() { ecs::set_camera_priority(entity_name, priority); });
			}
			{
				auto cmd = ecs_cmd->add_subcommand("add_camera_trauma", "Add trauma to a camera");
				static std::string entity_name;
				static float trauma;
				cmd->add_option("entity_name", entity_name, "The name of the camera entity")
					->required();
				cmd->add_option("trauma", trauma, "The amount of trauma to add")
					->required();
				cmd->callback([]() { ecs::add_camera_trauma(entity_name, trauma); });
			}
		}
	}
}
