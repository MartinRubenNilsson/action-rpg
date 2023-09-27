#include "ui_textbox.h"
#include <RmlUi/Core.h>
#include "ui.h"
#include "tables.h"
#include "audio.h"

namespace ui
{
	// Returns true if the character at rml_pos is plain text,
	// defined as a character that is not part of an RML tag.
	bool _is_plain_text(const std::string& rml, size_t rml_pos)
	{
		for (size_t i = rml_pos; i < rml.size(); ++i)
		{
			if (rml[i] == '<') return i != rml_pos;
			if (rml[i] == '>') return false;
		}
		return true;
	}

	// Returns the number of plain text characters.
	size_t _get_plain_text_length(const std::string& rml)
	{
		size_t count = 0;
		for (size_t i = 0; i < rml.size(); ++i)
			if (_is_plain_text(rml, i))
				++count;
		return count;
	}

	// Returns the plain text character at plain_text_pos,
	// or '\0' if plain_text_pos >= _get_plain_text_length(rml).
	char _get_plain_text_char(const std::string& rml, size_t plain_text_pos)
	{
		size_t current_plain_text_pos = 0;
		for (size_t i = 0; i < rml.size(); ++i)
		{
			if (_is_plain_text(rml, i))
			{
				if (current_plain_text_pos == plain_text_pos)
					return rml[i];
				++current_plain_text_pos;
			}
		}
		return '\0';
	}

	// Replaces all graphical plain text characters from plain_text_pos to the
	// end of the string with non-breaking spaces. This is used to prevent the
	// text from jumping around when it is being typed out.
	std::string _replace_graphical_plain_text_with_nbsp(
		const std::string& rml, size_t plain_text_pos)
	{
		std::string result;
		size_t current_plain_text_pos = 0;
		for (size_t i = 0; i < rml.size(); ++i)
		{
			std::string current_char = rml.substr(i, 1);
			if (_is_plain_text(rml, i))
			{
				if (current_plain_text_pos >= plain_text_pos && isgraph(rml[i]))
					current_char = "&nbsp;";
				++current_plain_text_pos;
			}
			result += current_char;
		}
		return result;
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

	std::deque<tables::TextboxEntry> _queued_entries;
	std::optional<tables::TextboxEntry> _current_entry;
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

		if (_current_entry->speed > 0.f)
		{
			float seconds_per_char = 1.f / _current_entry->speed;
			_typing_time_accumulator += dt;
			if (_typing_time_accumulator >= seconds_per_char)
			{
				_typing_time_accumulator -= seconds_per_char;
				if (isgraph(_get_plain_text_char(_textbox_text, _current_plain_text_length)))
					audio::play("event:/" + _current_entry->sound);
				++_current_plain_text_length;
			}
		}
		else
		{
			_current_plain_text_length = max_length;
		}

		_textbox_text = _replace_graphical_plain_text_with_nbsp(
			_textbox_text, _current_plain_text_length);
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
		auto entries = tables::get_textbox_entries(name);
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