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

	struct ArgTypenameVisitor
	{
		std::string operator()(std::monostate) const { return ""; }
		std::string operator()(bool) const { return "BOOL"; }
		std::string operator()(int) const { return "INT"; }
		std::string operator()(float) const { return "FLOAT"; }
		std::string operator()(const std::string&) const { return "STRING"; }
	};

	struct ArgParserVisitor
	{
		std::istream& is;

		void operator()(std::monostate&) {}

		void operator()(bool& value) { is >> std::boolalpha >> value >> std::noboolalpha; }

		void operator()(int& value) { is >> value; }

		void operator()(float& value) { is >> value; }

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

	using Params = std::array<Param, 8>;

	struct Command
	{
		const char* name = "";
		const char* desc = "";
		Params params{};
		void (*callback)(const Params& params) = nullptr;
	};

	bool operator<(const Command& left, const Command& right) {
		return strcmp(left.name, right.name) < 0;
	}

	std::vector<Command> _commands;

	void _initialize_commands()
	{
		_commands.clear();

		// CONSOLE
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "clear";
			cmd.desc = "Clears the console";
			cmd.callback = [](const Params&) { console::clear(); };
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "sleep";
			cmd.desc = "Defers incoming commands, executing them later";
			cmd.params[0] = { 0.f, "seconds", "The number of seconds to sleep" };
			cmd.callback = [](const Params& params) {
				console::sleep(std::get<float>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "log";
			cmd.desc = "Logs a message to the console";
			cmd.params[0] = { "", "message", "The message to log"};
			cmd.callback = [](const Params& params) {
				console::log(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "log_error";
			cmd.desc = "Logs an error message to the console";
			cmd.params[0] = { "", "message", "The message to log"};
			cmd.callback = [](const Params& params) {
				console::log_error(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "execute";
			cmd.desc = "Executes a console command";
			cmd.params[0] = { "", "command_line", "The command to execute" };
			cmd.callback = [](const Params& params) {
				console::execute(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "execute_script";
			cmd.desc = "Executes a console script";
			cmd.params[0] = { "", "script_name", "The name of the script"};
			cmd.callback = [](const Params& params) {
				console::execute_script(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "bind";
			cmd.desc = "Binds a console command to a key";
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
			cmd.desc = "Unbinds a key";
			cmd.params[0] = { "", "key", "The key to unbind" };
			cmd.callback = [](const Params& params) {
				console::unbind(std::get<std::string>(params[0].arg));
			};
		}
		
		// SHADERS
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "reload_shaders";
			cmd.desc = "Reloads all shaders";
			cmd.callback = [](const Params& params) { shaders::reload_assets(); };
		}

		// AUDIO
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "audio_play";
			cmd.desc = "Plays an audio event";
			cmd.params[0] = { "", "event_path", "The full path of the event" };
			cmd.callback = [](const Params& params) {
				audio::play(std::get<std::string>(params[0].arg));
			};
		}

		// MAP
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "map_open";
			cmd.desc = "Opens a map";
			cmd.params[0] = { "", "name", "The name of the map" };
			cmd.callback = [](const Params& params) {
				map::open(std::get<std::string>(params[0].arg));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "map_close";
			cmd.desc = "Closes the current map";
			cmd.callback = [](const Params& params) { map::close(); };
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "map_reset";
			cmd.desc = "Resets the current map";
			cmd.callback = [](const Params& params) { map::reset(); };
		}

		// UI
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "ui_show";
			cmd.desc = "Shows an RML document";
			cmd.params[0] = { "", "name", "The name of the document" };
			cmd.callback = [](const Params& params) {
				ui::show_document(std::get<std::string>(params[0].arg));
			};
		}

		// GAME
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "kill_player";
			cmd.desc = "Kills the player";
			cmd.callback = [](const Params&) {
				ecs::kill_player(ecs::find_entity_by_class("player"));
			};
		}
		{
			Command& cmd = _commands.emplace_back();
			cmd.name = "add_camera_shake";
			cmd.desc = "Adds trauma to the active camera to make it shake";
			cmd.params[0] = { 0.f, "trauma", "The amount of trauma to add" };
			cmd.callback = [](const Params& params) {
				ecs::add_trauma_to_active_camera(std::get<float>(params[0].arg));
			};
		}

		std::sort(_commands.begin(), _commands.end());
	}

	std::string _get_command_help_message(const Command& command)
	{
		std::string msg;
		msg += command.name;
		for (const Param& param : command.params) {
			if (std::holds_alternative<std::monostate>(param.arg)) break;
			msg += " [";
			msg += std::visit(ArgTypenameVisitor{}, param.arg);
			msg += " ";
			msg += param.name;
			msg += "]";
		}
		msg += "\n\t";
		msg += command.desc;
		for (const Param& param : command.params) {
			if (std::holds_alternative<std::monostate>(param.arg)) break;
			msg += "\n\t";
			msg += param.name;
			msg += ": ";
			msg += param.desc;
		}
		return msg;
	}

	void _execute_command(const std::string& command_line)
	{
		std::istringstream iss(command_line);
		std::string name;
		if (!(iss >> name)) return;
		bool help = (name == "help");
		if (help && !(iss >> name)) return;
		auto it = std::lower_bound(_commands.begin(), _commands.end(), Command{ name.c_str() });
		if (it == _commands.end() || it->name != name) {
			log_error("Unknown command: " + name);
			return;
		}
		if (help) {
			log(_get_command_help_message(*it));
			return;
		}
		if (!it->callback) {
			log_error("Command not implemented: " + name);
			return;
		}
		Params params = it->params;
		ArgParserVisitor arg_parser_visitor{ iss };
		for (Param& param : params) {
			std::visit(arg_parser_visitor, param.arg);
			if (iss) continue;
			log_error("Invalid argument: " + std::string(param.name));
			return;
		}
		it->callback(params);
	}

	std::vector<std::string> _complete_command(const std::string& prefix)
	{
		struct Compare
		{
			size_t size = 0;

			bool operator()(const Command& left, const Command& right) const {
				return strncmp(left.name, right.name, size) < 0;
			}
		};
		auto [begin, end] = std::equal_range(_commands.begin(), _commands.end(),
			Command{ prefix.c_str() }, Compare{ prefix.size() });
		std::vector<std::string> matches;
		for (auto it = begin; it != end; ++it) {
			matches.push_back(it->name);
		}
		return matches;
	}
}
