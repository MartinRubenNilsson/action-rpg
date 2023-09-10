#include "ui_textbox.h"
#include <RmlUi/Core.h>
#include "ui.h"
#include "data.h"

namespace ui
{
	// Returns true if the character at pos is a plain text character,
	// defined as a character that is not part of an RML tag.
	bool _is_plain_text(const std::string& rml, size_t pos)
	{
		for (size_t i = pos; i < rml.size(); ++i)
		{
			if (rml[i] == '<') return i != pos;
			if (rml[i] == '>') return false;
		}
		return true;
	}

	// Returns the number of plain text characters in the given RML string.
	size_t _get_plain_text_length(const std::string& rml)
	{
		size_t count = 0;
		for (size_t i = 0; i < rml.size(); ++i)
			if (_is_plain_text(rml, i))
				++count;
		return count;
	}

	// Replaces all plain text characters after the given count with spaces.
	void _create_plain_text_substr(std::string& rml, size_t count)
	{
		size_t current_count = 0;
		for (size_t i = 0; i < rml.size(); ++i)
		{
			if (_is_plain_text(rml, i))
			{
				if (current_count < count)
					++current_count;
				else
					rml[i] = ' ';
			}
		}
	}

	extern Rml::Context* _context;

	// These variables and functions are bound to the data model in ui_bindings.cpp.

	std::string _textbox_text; // RML string
	std::string _textbox_sprite;
	bool _textbox_sprite_is_set = false;

	void _on_textbox_keydown(int key)
	{
		if (!is_textbox_visible())
			return;

		if (key == Rml::Input::KI_C)
			if (!is_textbox_typing())
				advance_textbox_entry();

		if (key == Rml::Input::KI_X)
			if (is_textbox_typing())
				skip_textbox_typing();
			else
				advance_textbox_entry();
	}

	// These variables are only to be used internally in this file.

	std::deque<data::TextboxEntry> _queued_entries;
	std::optional<data::TextboxEntry> _current_entry;
	size_t _current_plain_text_length = 0;
	float _typing_time_accumulator = 0.f;

	void update_textbox(float dt)
	{
		auto doc = _context->GetDocument("textbox");
		if (!doc) return;

		if (!_current_entry && !_queued_entries.empty())
			advance_textbox_entry();

		if (!_current_entry)
		{
			doc->Hide();
			return;
		}

		doc->Show();

		_textbox_text = _current_entry->text;
		_textbox_sprite = _current_entry->sprite;
		_textbox_sprite_is_set = !_textbox_sprite.empty();

		size_t max_length = _get_plain_text_length(_textbox_text);

		if (_current_plain_text_length >= max_length)
		{
			_current_plain_text_length = max_length;
			_typing_time_accumulator = 0.f;
			return;
		}

		if (_current_entry->typing_speed > 0.f)
		{
			float seconds_per_char = 1.f / _current_entry->typing_speed;
			_typing_time_accumulator += dt;
			if (_typing_time_accumulator >= seconds_per_char)
			{
				_typing_time_accumulator -= seconds_per_char;
				++_current_plain_text_length;
			}
		}
		else
		{
			_current_plain_text_length = max_length;
		}

		_create_plain_text_substr(_textbox_text, _current_plain_text_length);
	}

	bool is_textbox_visible()
	{
		if (auto doc = _context->GetDocument("textbox"))
			return doc->IsVisible();
		return false;
	}

	bool is_textbox_typing() {
		return _current_plain_text_length <
			_get_plain_text_length(_textbox_text);
	}

	void skip_textbox_typing() {
		_current_plain_text_length =
			_get_plain_text_length(_textbox_text);
	}

	void advance_textbox_entry()
	{
		_current_entry.reset();
		_current_plain_text_length = 0;
		_typing_time_accumulator = 0.f;
		if (_queued_entries.empty()) return;
		_current_entry = _queued_entries.front();
		_queued_entries.pop_front();
	}

	void add_textbox_entries(const std::string& name)
	{
		auto entries = data::get_textbox_entries(name);
		std::ranges::copy(entries, std::back_inserter(_queued_entries));
	}

	void set_textbox_entries(const std::string& name)
	{
		_queued_entries.clear();
		add_textbox_entries(name);
	}

	// float opacity = doc->GetProperty("opacity")->Get<float>();
	// doc->Animate("opacity", Rml::Property(1.0f, Rml::Property::NUMBER), FADE_DURATION);
}