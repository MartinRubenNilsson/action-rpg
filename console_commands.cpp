#include "console.h"
#include <CLI/CLI.hpp>
#include "window.h"
#include "audio.h"
#include "map.h"
#include "ui.h"
#include "ui_textbox.h"

namespace console
{
	void _setup_commands(CLI::App& app)
	{
		// GENERAL
		{
			app.add_subcommand("exit", "Exit the game")
				->callback(window::close);
			app.add_subcommand("clear", "Clear the console")
				->callback(console::clear);
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
			cmd->add_subcommand("open", "Open a map")
				->add_option_function<std::string>("name", map::open, "The name of the map")
				->required();
			cmd->add_subcommand("close", "Close the current map")
				->callback(map::close);
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
	}
}
