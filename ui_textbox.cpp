#include "ui_textbox.h"
#include <RmlUi/Core.h>
#include "ui.h"
#include "console.h"

#define FADE_DURATION 0.125f
#define TIME_PER_CHAR 0.025f

namespace ui
{
	bool TextboxEntry::empty() const {
		return text.empty() && sprite.empty();
	}

	extern Rml::Context* _context;

	// These variables are bound to the data model in ui_bindings.cpp.

	std::string _textbox_text; // RML markdown
	std::string _textbox_sprite;
	bool _textbox_sprite_is_set = false;

	// These variables are used internally in this file.

	static std::deque<TextboxEntry> _queued_entries;
	static TextboxEntry _current_entry;
	static size_t _plain_text_chars_to_show = 0;
	static float _time_accumulator = 0.f;

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

	size_t _get_num_plain_text_chars(const std::string& rml)
	{
		size_t count = 0;
		for (size_t i = 0; i < rml.size(); ++i)
		{
			if (_is_plain_text_char(rml, i))
				++count;
		}
		return count;
	}

	// Replaces all but the first n plain text characters with spaces.
	// RML tags are left untouched.
	void _show_n_plain_text_chars(std::string& rml, size_t n)
	{
		size_t count = 0;
		for (size_t i = 0; i < rml.size(); ++i)
		{
			if (_is_plain_text_char(rml, i))
			{
				if (count < n)
					++count;
				else
					rml[i] = ' ';
			}
		}
	}

	void _on_textbox_keydown(int key)
	{
		if (!is_textbox_visible())
			return;

		if (key == Rml::Input::KI_C)
			if (!is_textbox_typing())
				pop_textbox_entry();

		if (key == Rml::Input::KI_X)
			skip_textbox_typing();
	}

	void update_textbox(float dt)
	{
		auto doc = _context->GetDocument("textbox");
		if (!doc) return;

		if (_current_entry.empty())
		{
			doc->Hide();
			return;
		}

		doc->Show();

		_textbox_text = _current_entry.text;
		_textbox_sprite = _current_entry.sprite;
		_textbox_sprite_is_set = !_textbox_sprite.empty();

		size_t plain_text_chars = _get_num_plain_text_chars(_textbox_text);

		if (_plain_text_chars_to_show >= plain_text_chars)
		{
			_plain_text_chars_to_show = plain_text_chars;
			_time_accumulator = 0.f;
			return;
		}

		_time_accumulator += dt;
		if (_time_accumulator >= TIME_PER_CHAR)
		{
			_time_accumulator -= TIME_PER_CHAR;
			++_plain_text_chars_to_show;
		}
		
		_show_n_plain_text_chars(_textbox_text, _plain_text_chars_to_show);
	}

	bool is_textbox_visible()
	{
		if (auto doc = _context->GetDocument("textbox"))
			return doc->IsVisible();
		return false;
	}

	bool is_textbox_typing() {
		return _plain_text_chars_to_show <
			_get_num_plain_text_chars(_current_entry.text);
	}

	void skip_textbox_typing() {
		_plain_text_chars_to_show =
			_get_num_plain_text_chars(_current_entry.text);
	}

	//void open_textbox()
	//{
	// float opacity = doc->GetProperty("opacity")->Get<float>();
	//	if (auto doc = _context->GetDocument("textbox"))
	//	{
	//		doc->Animate("opacity", Rml::Property(1.0f, Rml::Property::NUMBER), FADE_DURATION);
	//		// TODO: Play sound.
	//	}
	//}
	//
	//void close_textbox()
	//{
	//	if (auto doc = _context->GetDocument("textbox"))
	//	{
	//		doc->Animate("opacity", Rml::Property(0.0f, Rml::Property::NUMBER), FADE_DURATION);
	//		// TODO: Play sound.
	//	}
	//}

	void push_textbox_entry(const TextboxEntry& entry) {
		_queued_entries.push_back(entry);
	}

	void pop_textbox_entry()
	{
		_current_entry = {};
		_plain_text_chars_to_show = 0;
		_time_accumulator = 0.f;
		if (_queued_entries.empty()) return;
		_current_entry = _queued_entries.front();
		_queued_entries.pop_front();
	}

	void set_textbox_text(const std::string& text) {
		_current_entry.text = text;
	}
	
	void set_textbox_sprite(const std::string& sprite) {
		_current_entry.sprite = sprite;
	}
}