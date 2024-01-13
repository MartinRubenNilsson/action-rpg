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
				auto cmd = app.add_subcommand("bind", "Bind a key to a console command");
				cmd->add_option("key", key_string, "The key to bind")
					->required();
				cmd->add_option("command", command_line, "The command to execute")
					->required();
				cmd->callback([]() { console::bind(key_string, command_line); });
			}
			{
				static std::string key_string;
				auto cmd = app.add_subcommand("unbind", "Unbind a key");
				cmd->add_option("key", key_string, "The key to unbind")
					->required();
				cmd->callback([]() { console::unbind(key_string); });
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
			//cmd->add_subcommand("stop_all", "Stop all audio events")
			//	->callback(audio::stop_all);
		}

		// MAP
		{
			auto map_cmd = app.add_subcommand("map", "Manage maps");
			//map_cmd->add_subcommand("reload_textures", "Reload all textures in use by any map")
			//	->callback(map::reload_textures);
			{
				auto cmd = map_cmd->add_subcommand("open", "Open a map");
				static std::string map_name;
				static bool force = false;
				cmd->add_option("map_name", map_name, "The name of the map")->required();
				cmd->add_flag("-f,--force", force, "Force the map to open");
				cmd->callback([]() { map::open(map_name, force); });
			}
			map_cmd->add_subcommand("close", "Close the current map")
				->callback(map::close);
			map_cmd->add_subcommand("reset", "Reset the current map")
				->callback(map::reset);
			{
				auto cmd = map_cmd->add_subcommand("spawn", "Spawn an entity from a template");
				static std::string template_name;
				static map::SpawnOptions options;
				cmd->add_option("template_name", template_name, "The name of the template")->required();
				cmd->add_option("-x", options.x, "The x position of the entity");
				cmd->add_option("-y", options.y, "The y position of the entity");
				cmd->add_option("-z", options.z, "The z position of the entity");
				cmd->callback([]() { map::spawn(template_name, options); });
			}
			map_cmd->add_subcommand("spawnpoint", "Set the player spawnpoint entity")
				->add_option_function<std::string>("name", map::set_player_spawnpoint,
					"The name of the spawnpoint entity")
				->required();
		}

		// DATA
		{
			auto data_cmd = app.add_subcommand("data", "Manage data");
			data_cmd->add_subcommand("clear", "Clear all data")
				->callback(data::clear);
			data_cmd->add_subcommand("load", "Load data from a JSON file")
				->add_option_function<std::string>("name", data::load, "The name of the file")
				->required();
			data_cmd->add_subcommand("save", "Save data to a JSON file")
				->add_option_function<std::string>("name", data::save, "The name of the file")
				->required();
			data_cmd->add_subcommand("dump", "Dump all data as a string to the console")
				->callback([]() { log(data::dump()); });
			{
				auto cmd = data_cmd->add_subcommand("set_bool", "Set a data bool");
				static std::string key;
				static bool value;
				cmd->add_option("key", key, "The key of the data")->required();
				cmd->add_option("value", value, "The value of the data")->required();
				cmd->callback([]() { data::set_bool(key, value); });
			}
			{
				auto cmd = data_cmd->add_subcommand("get_bool", "Get a data bool");
				static std::string key;
				cmd->add_option("key", key, "The key of the data")->required();
				cmd->callback([]() {
					bool value;
					if (data::get_bool(key, value)) log(std::to_string(value));
					else log_error("Failed to get data bool: " + key);
				});
			}
			{
				auto cmd = data_cmd->add_subcommand("set_int", "Set a data int");
				static std::string key;
				static int value;
				cmd->add_option("key", key, "The key of the data")->required();
				cmd->add_option("value", value, "The value of the data")->required();
				cmd->callback([]() { data::set_int(key, value); });
			}
			{
				auto cmd = data_cmd->add_subcommand("get_int", "Get a data int");
				static std::string key;
				cmd->add_option("key", key, "The key of the data")->required();
				cmd->callback([]() {
					int value;
					if (data::get_int(key, value)) log(std::to_string(value));
					else log_error("Failed to get data int: " + key);
				});
			}
			{
				auto cmd = data_cmd->add_subcommand("set_float", "Set a data float");
				static std::string key;
				static float value;
				cmd->add_option("key", key, "The key of the data")->required();
				cmd->add_option("value", value, "The value of the data")->required();
				cmd->callback([]() { data::set_float(key, value); });
			}
			{
				auto cmd = data_cmd->add_subcommand("get_float", "Get a data float");
				static std::string key;
				cmd->add_option("key", key, "The key of the data")->required();
				cmd->callback([]() {
					float value;
					if (data::get_float(key, value)) log(std::to_string(value));
					else log_error("Failed to get data float: " + key);
				});
			}
			{
				auto cmd = data_cmd->add_subcommand("set_string", "Set a data string");
				static std::string key;
				static std::string value;
				cmd->add_option("key", key, "The key of the data")->required();
				cmd->add_option("value", value, "The value of the data")->required();
				cmd->callback([]() { data::set_string(key, value); });
			}
			{
				auto cmd = data_cmd->add_subcommand("get_string", "Get a data string");
				static std::string key;
				cmd->add_option("key", key, "The key of the data")->required();
				cmd->callback([]() {
					std::string value;
					if (data::get_string(key, value)) log(value);
					else log_error("Failed to get data string: " + key);
				});
			}
		}

		// UI
		{
			auto ui_cmd = app.add_subcommand("ui", "Manage UI");
			ui_cmd->add_subcommand("reload_styles", "Reload all style sheets")
				->callback(ui::reload_styles);
			ui_cmd->add_subcommand("show", "Show a document")
				->add_option_function<std::string>("name", ui::show_document, "The name of the document")
				->required();
			ui_cmd->add_subcommand("hide", "Hide a document")
				->add_option_function<std::string>("name", ui::hide_document, "The name of the document")
				->required();
			ui_cmd->add_subcommand("open_textbox_preset", "Open a textbox preset")
				->add_option_function<std::string>("name", ui::open_textbox_preset, "The name of the textbox")
				->required();
		}

		// ECS
		{
			auto ecs_cmd = app.add_subcommand("ecs", "Manage the ECS");
			ecs_cmd->add_subcommand("kill_player", "Kill the player")
				->callback(ecs::kill_player);
			{
				auto cmd = ecs_cmd->add_subcommand("hurt_player", "Hurt the player");
				static int health_to_remove;
				cmd->add_option("health_to_remove", health_to_remove, "The amount of health to remove")
					->required();
				cmd->callback([]() { ecs::hurt_player(health_to_remove); });
			}
			{
				auto cmd = ecs_cmd->add_subcommand("activate_camera", "Activate a camera");
				static std::string entity_name;
				cmd->add_option("entity_name", entity_name, "The name of the camera entity")
					->required();
				cmd->callback([]() { ecs::activate_camera(entity_name); });
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
