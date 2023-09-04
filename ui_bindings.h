#pragma once

namespace ui
{
	// DATA VARIABLES

	extern std::string test_string;

	// EVENT CALLBACKS

	extern void (*console_log)(const std::string& message);

	void create_bindings();
	bool is_variable_dirty(const std::string& name);
	void dirty_all_variables(); // Called by ui::update().
}

