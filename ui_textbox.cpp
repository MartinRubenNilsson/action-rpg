#include "ui_textbox.h"
#include <RmlUi/Core.h>
#include "console.h"

namespace ui
{
	extern Rml::Context* _context;

	std::string _textbox_text; // Technically not text, but RML.
	std::string _textbox_sprite;
	bool _textbox_sprite_is_set = false;

	void _on_textbox_keydown(int key) {
		console::log_error("Bruh " + std::to_string(key));
	}

	void set_textbox_text(const std::string& text) {
		_textbox_text = text;
	}
	 
	void set_textbox_sprite(const std::string& sprite) {
		_textbox_sprite = sprite;
		_textbox_sprite_is_set = !sprite.empty();
	}
}