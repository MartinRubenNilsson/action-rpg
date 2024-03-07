#pragma once

namespace console
{
	using Arg = std::variant<std::monostate, bool, int, float, std::string, sf::Vector2f>;

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

	std::string format_help_message(const Command& command);

	void register_commands();
	const Command* find_command(const std::string& name);
	void parse_and_execute_command(const std::string& command_line);
	std::vector<std::string> complete_command(const std::string& prefix);
}