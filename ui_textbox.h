#pragma once

namespace ui
{
	void update_textbox(float dt);

	bool is_textbox_open();
	void open_textbox();
	void close_textbox();

	// Technically not text, but RML.
	void set_textbox_text(const std::string& text);
	
	// Empty string hides the sprite.
	void set_textbox_sprite(const std::string& sprite);
}
