#include "console.h"
#include <CLI/CLI.hpp>
#include "map.h"
#include "rml.h"
#include "audio.h"

extern void close_window();

namespace console
{
	void _setup_commands(CLI::App& app)
	{
		app.add_subcommand("exit", "Exit the game")->callback(close_window);
		app.add_subcommand("clear", "Clear the console")->callback(clear);

		// MAP
		{
			auto cmd = app.add_subcommand("map", "Handle maps");
			cmd->add_subcommand("list", "List all loaded maps")
				->callback([]() { for (const auto& name : map::get_list()) log(name); });
			cmd->add_subcommand("name", "Print the name of the current map")
				->callback([]() { log(map::get_name()); });
			cmd->add_subcommand("open", "Open a map")
				->add_option_function<std::string>("name", map::open, "The name of the map");
			cmd->add_subcommand("close", "Close the current map")
				->callback(map::close);
		}

		// RML
		{
			auto cmd = app.add_subcommand("rml", "Handle RML documents");
			cmd->add_subcommand("list", "List all loaded documents")
				->callback([]() { for (const auto& name : rml::get_list()) log(name); });
			cmd->add_subcommand("show", "Show a document")
				->add_option_function<std::string>("name", rml::show, "The name of the document");
			cmd->add_subcommand("hide", "Hide a document")
				->add_option_function<std::string>("name", rml::hide, "The name of the document");
		}

		// SOUND
		{
			auto cmd = app.add_subcommand("sound", "Handle sounds");
			cmd->add_subcommand("play", "Play a sound")
				->add_option_function<std::string>("name", audio::play_sound, "The name of the sound");
		}
	}
}
