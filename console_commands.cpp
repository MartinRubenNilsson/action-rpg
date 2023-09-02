#include "console.h"
#include <CLI/CLI.hpp>
#include "audio.h"
#include "map.h"
#include "ui.h"

namespace console
{
	void _setup_commands(CLI::App& app)
	{
		// AUDIO
		{
			auto cmd = app.add_subcommand("audio", "Manage audio");
			cmd->add_subcommand("play", "Play an audio event")
				->add_option_function<std::string>("name", audio::play_sound, "The name of the sound event")
				->required();
		}

		// MAP
		{
			auto cmd = app.add_subcommand("map", "Manage maps");
			cmd->add_subcommand("list", "List all loaded maps")
				->callback([]() { for (const auto& name : map::get_loaded_maps()) log(name); });
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
			auto cmd = app.add_subcommand("ui", "Manage UI documents");
			cmd->add_subcommand("reload", "Reload all style sheets")
				->callback(ui::reload_style_sheets);
			cmd->add_subcommand("list", "List all loaded documents")
				->callback([]() { for (const auto& name : ui::get_loaded_documents()) log(name); });
			cmd->add_subcommand("show", "Show a document")
				->add_option_function<std::string>("name", ui::show, "The name of the document")
				->required();
			cmd->add_subcommand("hide", "Hide a document")
				->add_option_function<std::string>("name", ui::hide, "The name of the document")
				->required();
		}
	}
}
