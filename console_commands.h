#pragma once

namespace console
{
	using Arg = std::variant<std::monostate, bool, int, float, std::string, Vector2f>;

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

	using CommandIt = std::vector<Command>::const_iterator;

	std::string format_type_and_name(const Param& param);
	std::string format_help_message(const Command& command);

	void register_commands();
	void register_commands_misc(std::vector<Command>& commands);
	void register_commands_steam(std::vector<Command>& commands);

	void parse_and_execute_command(const std::string& command_line);
	CommandIt commands_begin();
	CommandIt commands_end();
	CommandIt find_command(const std::string& name);
	// Returns a consecutive range of commands starting with the given prefix.
	std::pair<CommandIt, CommandIt> find_commands_starting_with(const std::string& prefix);
}