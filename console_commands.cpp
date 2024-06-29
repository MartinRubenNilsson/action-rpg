#include "stdafx.h"
#include "console_commands.h"
#include "console.h"

namespace console
{
	struct ArgTypenameVisitor
	{
		std::string operator()(std::monostate) const { return ""; }
		std::string operator()(bool) const { return "BOOL"; }
		std::string operator()(int) const { return "INT"; }
		std::string operator()(float) const { return "FLOAT"; }
		std::string operator()(const std::string&) const { return "STRING"; }
		std::string operator()(const Vector2f&) const { return "VEC2F"; }
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
			if (is.peek() == '"') {
				is.ignore();
				std::getline(is, value, '"');
			} else {
				is >> value;
			}
		}
		void operator()(Vector2f& value) { is >> value.x >> value.y; }
	};

	std::vector<Command> _commands; // Sorted by name

	bool operator<(const Command& left, const Command& right) {
		return strcmp(left.name, right.name) < 0; // Order by name
	}

	std::string format_type_and_name(const Param& param)
	{
		std::string ret;
		ret += std::visit(ArgTypenameVisitor{}, param.arg);
		ret += " ";
		ret += param.name;
		return ret;
	}

	std::string format_help_message(const Command& command)
	{
		std::string ret;
		ret += command.name;
		for (const Param& param : command.params) {
			if (std::holds_alternative<std::monostate>(param.arg)) break;
			ret += " [" + format_type_and_name(param) + "]";
		}
		ret += "\n- ";
		ret += command.desc;
		for (const Param& param : command.params) {
			if (std::holds_alternative<std::monostate>(param.arg)) break;
			ret += "\n- ";
			ret += param.name;
			ret += ": ";
			ret += param.desc;
		}
		return ret;
	}

	void register_commands()
	{
		_commands.clear();
		register_commands_misc(_commands);
		register_commands_steam(_commands);
		std::sort(_commands.begin(), _commands.end());
	}

	void parse_and_execute_command(const std::string& command_line)
	{
		std::istringstream iss(command_line);
		std::string name;
		if (!(iss >> name)) return;
		CommandIt cmd = find_command(name);
		if (cmd == _commands.end()) return;
		if (!cmd->callback) {
			log_error("Command not implemented: " + name);
			return;
		}
		Params params = cmd->params; // Intentional copy
		for (Param& param : params) {
			if (std::holds_alternative<std::monostate>(param.arg)) break;
			iss.ignore(64, ' '); // Skip any leading spaces
			if (iss.eof()) {
				log_error("Missing argument: " + format_type_and_name(param));
				return;
			}
			std::visit(ArgParserVisitor{ iss }, param.arg);
			if (iss.fail()) {
				log_error("Invalid argument: " + format_type_and_name(param));
				return;
			}
		}
		cmd->callback(params);
	}

	CommandIt commands_begin() {
		return _commands.begin();
	}

	CommandIt commands_end() {
		return _commands.end();
	}

	CommandIt find_command(const std::string& name)
	{
		// Do a binary search to find the command. We assume the commands are sorted by name.
		CommandIt it = std::lower_bound(_commands.begin(), _commands.end(), Command{ name.c_str() });
		if (it != _commands.end() && it->name == name) return it;
		log_error("Unknown command: " + name);
		return _commands.end();
	}

	std::pair<CommandIt, CommandIt> find_commands_starting_with(const std::string& prefix)
	{
		const size_t prefix_size = prefix.size();
		return std::equal_range(_commands.begin(), _commands.end(), Command{ prefix.c_str() },
			[prefix_size](const Command& left, const Command& right) {
				return strncmp(left.name, right.name, prefix_size) < 0;
			}
		);
	}
}
