#pragma once

namespace ui
{
	// DATA VARIABLES

	extern std::string textbox_sprite;
	extern std::string textbox_text; // Technically not text, but RML.

	// EVENT CALLBACKS

	extern void (*console_log)(const std::string& message);

	// INTERFACE - DON'T TOUCH

	void create_bindings();
	bool is_variable_dirty(const std::string& name);
	void dirty_all_variables(); // Called automatically by ui::update().
}

