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
		std::string operator()(const sf::Vector2f&) const { return "VEC2F"; }
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
		void operator()(sf::Vector2f& value) { is >> value.x >> value.y; }
	};

	std::vector<Command> _commands; // Sorted by name

	std::string _format_type_and_name(const Param& param)
	{
		std::string ret;
		ret += std::visit(ArgTypenameVisitor{}, param.arg);
		ret += " ";
		ret += param.name;
		return ret;
	}
	
	bool operator<(const Command& left, const Command& right) {
		return strcmp(left.name, right.name) < 0; // Order by name
	}

	std::string format_help_message(const Command& command)
	{
		std::string ret;
		ret += command.name;
		for (const Param& param : command.params) {
			if (std::holds_alternative<std::monostate>(param.arg)) break;
			ret += " [" + _format_type_and_name(param) + "]";
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

	const Command* find_command(const std::string& name)
	{
		// Do a binary search to find the command. We assume the commands are sorted by name.
		auto it = std::lower_bound(_commands.begin(), _commands.end(), Command{ name.c_str() });
		if (it != _commands.end() && it->name == name) return &*it;
		log_error("Unknown command: " + name);
		return nullptr;
	}

	extern void _register_commands_misc(std::vector<Command>& commands);

	void register_commands()
	{
		_commands.clear();
		_register_commands_misc(_commands);
		std::sort(_commands.begin(), _commands.end());
	}

	void parse_and_execute_command(const std::string& command_line)
	{
		std::istringstream iss(command_line);
		std::string name;
		if (!(iss >> name)) return;
		const Command* cmd = find_command(name);
		if (!cmd) return;
		if (!cmd->callback) {
			log_error("Command not implemented: " + name);
			return;
		}
		Params params = cmd->params; // Intentional copy
		for (Param& param : params) {
			if (std::holds_alternative<std::monostate>(param.arg)) break;
			iss.ignore(64, ' '); // Skip any leading spaces
			if (iss.eof()) {
				log_error("Missing argument: " + _format_type_and_name(param));
				return;
			}
			std::visit(ArgParserVisitor{ iss }, param.arg);
			if (iss.fail()) {
				log_error("Invalid argument: " + _format_type_and_name(param));
				return;
			}
		}
		cmd->callback(params);
	}

	std::vector<std::string> complete_command(const std::string& prefix)
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
