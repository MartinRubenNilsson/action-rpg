#pragma once

// THIS FILE IS FOR INTERNAL USE ONLY.
// DO NOT INCLUDE OUTSIDE OF THE UI MODULE.

namespace ui
{
	// DATA VARIABLES

	extern std::string textbox_text; // Technically not text, but RML.
	extern std::string textbox_sprite;
	extern bool textbox_sprite_is_set;

	// EVENT CALLBACKS

	extern void (*console_log)(const std::string& message);

	// INTERFACE

	void create_bindings();
	bool is_variable_dirty(const std::string& name);
	void dirty_all_variables(); // Called automatically by ui::update().
}

