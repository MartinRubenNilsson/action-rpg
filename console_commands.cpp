#include "console.h"
#include <CLI/CLI.hpp>
#include "map.h"
#include "rml.h"
#include "audio.h"

namespace console
{
	void _setup_commands(CLI::App& app)
	{
		// SETUP SOUND COMMANDS
		{
			auto cmd = app.add_subcommand("sound", "Handle sounds");
			cmd->add_subcommand("play", "Play a sound")
				->add_option_function<std::string>("name", audio::play_sound, "The name of the sound");
		}

		// SETUP MAP COMMANDS
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

		// SETUP RML COMMANDS
		{
			auto cmd = app.add_subcommand("rml", "Handle RML documents");
			cmd->add_subcommand("list", "List all loaded documents")
				->callback([]() { for (const auto& name : rml::get_list()) log(name); });
			cmd->add_subcommand("show", "Show a document")
				->add_option_function<std::string>("name", rml::show, "The name of the document");
			cmd->add_subcommand("hide", "Hide a document")
				->add_option_function<std::string>("name", rml::hide, "The name of the document");
		}
	}
}
