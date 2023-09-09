#include "ui_textbox.h"
#include <RmlUi/Core.h>
#include "ui.h"
#include "console.h"

#define FADE_DURATION 0.125f
#define TIME_PER_CHAR 0.025f

namespace ui
{
	extern Rml::Context* _context;

	static float _time_accumulator = 0.f;
	static size_t _num_chars_to_show = 0;
	static std::string _text_to_show; // RML markdown

	std::string _textbox_text; // RML markdown
	std::string _textbox_sprite;
	bool _textbox_sprite_is_set = false;

	void _on_textbox_keydown(int key)
	{
		if (key == Rml::Input::KI_C)
		{
			if (is_textbox_open())
				close_textbox();
			else
				open_textbox();
		}
		if (key == Rml::Input::KI_X)
		{
			// TODO: Implement.
		}
	}

	// Returns true if the character at index is a plain text character.
	// RML tags are not counted as plain text characters.
	bool _is_plain_text_char(const std::string& rml, size_t index)
	{
		for (size_t i = index; i < rml.size(); ++i)
		{
			if (rml[i] == '<') return i != index;
			if (rml[i] == '>') return false;
		}
		return true;
	}

	// Replaces all but the first n plain text characters with spaces.
	// RML tags are left untouched.
	void _show_n_plain_text_chars(std::string& rml, size_t n)
	{
		size_t num_chars_shown = 0;
		for (size_t i = 0; i < rml.size(); ++i)
		{
			if (_is_plain_text_char(rml, i))
			{
				if (num_chars_shown < n)
					++num_chars_shown;
				else
					rml[i] = ' ';
			}
		}
	}

	void update_textbox(float dt)
	{
		auto doc = _context->GetDocument("textbox");
		if (!doc) return;
		doc->Hide();

		float opacity = doc->GetProperty("opacity")->Get<float>();
		if (opacity == 0) return;
		doc->Show();
		if (opacity < 1) return;

		_time_accumulator += dt;
		if (_time_accumulator < TIME_PER_CHAR) return;
		_time_accumulator -= TIME_PER_CHAR;
		++_num_chars_to_show;

		_textbox_text = _text_to_show;
		_show_n_plain_text_chars(_textbox_text, _num_chars_to_show);
	}

	bool is_textbox_open()
	{
		if (auto doc = _context->GetDocument("textbox"))
			return doc->IsVisible();
		return false;
	}

	void open_textbox()
	{
		if (auto doc = _context->GetDocument("textbox"))
		{
			doc->Animate("opacity", Rml::Property(1.0f, Rml::Property::NUMBER), FADE_DURATION);
			// TODO: Play sound.
		}
	}

	void close_textbox()
	{
		if (auto doc = _context->GetDocument("textbox"))
		{
			doc->Animate("opacity", Rml::Property(0.0f, Rml::Property::NUMBER), FADE_DURATION);
			// TODO: Play sound.
		}
	}

	void set_textbox_text(const std::string& text)
	{
		_time_accumulator = 0.f;
		_num_chars_to_show = 0;
		_text_to_show = text;
		_textbox_text.clear();
	}
	 
	void set_textbox_sprite(const std::string& sprite)
	{
		_textbox_sprite = sprite;
		_textbox_sprite_is_set = !sprite.empty();
	}
}