#include "console.h"
#include "window.h"
#include "audio.h"
#include "map.h"
#include "ui.h"
#include "ecs_player.h"
#include "ecs_common.h"
#include "ecs_camera.h"
#include "shaders.h"

namespace console
{
	using Arg = std::variant<std::monostate, bool, int, float, std::string>;

	struct ArgParser
	{
		std::istream& is;

		void operator()(std::monostate&) {}

		void operator()(bool& value) {
			is >> std::boolalpha >> value >> std::noboolalpha;
		}

		void operator()(int& value) {
			is >> value;
		}

		void operator()(float& value) {
			is >> value;
		}

		void operator()(std::string& value) {
			// Try to parse a string with quotes. If it fails, parse a string without quotes.
			is.ignore(64, ' ');
			if (is.peek() == '"') {
				is.ignore();
				std::getline(is, value, '"');
				return;
			}
			is >> value;
		}
	};

	struct Param
	{
		Arg arg{};
		const char* name = "";
		const char* desc = "";
	};

	const size_t _MAX_PARAMS = 8;
	using Params = std::array<Param, _MAX_PARAMS>;

	struct Command
	{
		const char* name = "";
		const char* desc = "";
		Params params{};
		void (*callback)(const Params& params) = nullptr;
	};

	std::vector<Command> _commands;

	bool _compare_commands(const Command& left, const Command& right) {
		return strcmp(left.name, right.name) < 0;
	}

	bool _compare_command_and_name(const Command& left, const std::string& name) {
		return strcmp(left.name, name.c_str()) < 0;
	}

	void _initialize_commands()
	{
		_commands.clear();

		// CONSOLE
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "clear";
			cmd.desc = "Clear the console";
			cmd.callback = [](const Params&) { console::clear(); };
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "sleep";
			cmd.desc = "Make the console sleep for a number of seconds";
			cmd.params[0] = { 0.f, "seconds", "The number of seconds to sleep" };
			cmd.callback = [](const Params& params) {
				console::sleep(std::get<float>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "log";
			cmd.desc = "Log a message to the console";
			cmd.params[0] = { "", "message", "The message to log"};
			cmd.callback = [](const Params& params) {
				console::log(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "log_error";
			cmd.desc = "Log an error message to the console";
			cmd.params[0] = { "", "message", "The message to log"};
			cmd.callback = [](const Params& params) {
				console::log_error(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "execute";
			cmd.desc = "Execute a console command";
			cmd.params[0] = { "", "command_line", "The command to execute" };
			cmd.callback = [](const Params& params) {
				console::execute(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "execute_script";
			cmd.desc = "Execute a console script";
			cmd.params[0] = { "", "script_name", "The name of the script"};
			cmd.callback = [](const Params& params) {
				console::execute_script(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "bind";
			cmd.desc = "Bind a console command to a key";
			cmd.params[0] = { "", "key", "The key to bind" };
			cmd.params[1] = { "", "command_line", "The command to execute" };
			cmd.callback = [](const Params& params) {
				console::bind(
					std::get<std::string>(params[0].arg),
					std::get<std::string>(params[1].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "unbind";
			cmd.desc = "Unbind a key";
			cmd.params[0] = { "", "key", "The key to unbind" };
			cmd.callback = [](const Params& params) {
				console::unbind(std::get<std::string>(params[0].arg));
			};
		}

		// CAMERA
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "add_camera_trauma";
			cmd.desc = "Add trauma to the active camera to make it shake";
			cmd.params[0] = { 0.f, "trauma", "The amount of trauma to add" };
			cmd.callback = [](const Params& params) {
				ecs::add_camera_trauma(std::get<float>(params[0].arg));
			};
		}
		
		// SHADERS
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "reload_shaders";
			cmd.desc = "Reload all shaders";
			cmd.callback = [](const Params& params) { shaders::reload_assets(); };
		}

		// AUDIO
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "audio_play";
			cmd.desc = "Play an audio event";
			cmd.params[0] = { "", "event_path", "The full path of the event" };
			cmd.callback = [](const Params& params) {
				audio::play(std::get<std::string>(params[0].arg));
			};
		}

		// MAP
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "map_open";
			cmd.desc = "Open a map";
			cmd.params[0] = { "", "map_name", "The name of the map" };
			cmd.callback = [](const Params& params) {
				map::open(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "map_close";
			cmd.desc = "Close the current map";
			cmd.callback = [](const Params& params) { map::close(); };
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "map_reset";
			cmd.desc = "Reset the current map";
			cmd.callback = [](const Params& params) { map::reset(); };
		}

		// UI
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "ui_show";
			cmd.desc = "Show an RML document";
			cmd.params[0] = { "", "name", "The name of the document" };
			cmd.callback = [](const Params& params) {
				ui::show_document(std::get<std::string>(params[0].arg));
			};
		}

		// MISC
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "kill_player";
			cmd.desc = "Kill the player";
			cmd.callback = [](const Params&) {
				ecs::kill_player(ecs::find_entity_by_class("player"));
			};
		}

		// Sort the commands by name.
		std::sort(_commands.begin(), _commands.end(), _compare_commands);
	}

	void _log_command_help(const Command& command)
	{
		std::string line = command.name;
		for (const Param& param : command.params) {
			//TODO
		}
		log(line);
	}

	void _execute_command(const std::string& command_line)
	{
		std::istringstream iss(command_line);
		std::string name;
		if (!(iss >> name)) return;

		bool help = (name == "help");
		if (help) {
			if (!(iss >> name)) return;
		}

		auto it = std::lower_bound(_commands.begin(), _commands.end(), name, _compare_command_and_name);
		if (it == _commands.end() || it->name != name) {
			log_error("Unknown command: " + name);
			return;
		}
		if (help) {
			_log_command_help(*it);
			return;
		}
		if (!it->callback) {
			log_error("Command not implemented: " + name);
			return;
		}

		Params params = it->params;
		ArgParser arg_parser{ iss };
		for (Param& param : params) {
			if (!iss) break;
			std::visit(arg_parser, param.arg);
			if (iss.fail()) {
				log_error("Invalid argument: " + std::string(param.name));
				return;
			}
		}

		it->callback(params);
	}
}
