#include "ui_textbox.h"
#include <RmlUi/Core.h>
#include "ui.h"

#define FADE_DURATION 0.15f

namespace ui
{
	extern Rml::Context* _context;

	std::string _textbox_text; // Technically not text, but RML.
	std::string _textbox_sprite;
	bool _textbox_sprite_is_set = false;

	void _on_textbox_keydown(int key) {
		if (key == Rml::Input::KI_C)
		{
			if (auto doc = _context->GetDocument("textbox"))
				doc->Animate("opacity", Rml::Property(0.0f, Rml::Property::NUMBER), FADE_DURATION);
		}
		if (key == Rml::Input::KI_X)
		{
			if (auto doc = _context->GetDocument("textbox"))
				doc->Animate("opacity", Rml::Property(1.0f, Rml::Property::NUMBER), FADE_DURATION);
		}
	}

	void set_textbox_text(const std::string& text) {
		_textbox_text = text;
	}
	 
	void set_textbox_sprite(const std::string& sprite) {
		_textbox_sprite = sprite;
		_textbox_sprite_is_set = !sprite.empty();
	}
}