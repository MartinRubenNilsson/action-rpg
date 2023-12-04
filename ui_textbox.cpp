#include "ui_textbox.h"
#include <RmlUi/Core.h>
#include "ui.h"
#include "audio.h"

namespace ui
{
	extern Rml::Context* _context;

	bool _textbox_is_open = false;
	Textbox _textbox;
	std::deque<Textbox> _textbox_queue;
	float _textbox_current_time = 0.f;
	size_t _textbox_current_length = 0;

	std::string _textbox_text; // RML
	std::string _textbox_sprite;
	bool _textbox_sprite_is_set = false;

	// Returns true if the character at pos is plain text,
	// defined as a character that is not part of an RML tag.
	bool _is_plain(const std::string& rml, size_t pos)
	{
		for (size_t i = pos; i < rml.size(); ++i) {
			if (rml[i] == '<') return i != pos;
			if (rml[i] == '>') return false;
		}
		return true;
	}

	// Counts the number of plain text characters in the string.
	size_t _get_plain_length(const std::string& rml)
	{
		size_t count = 0;
		for (size_t i = 0; i < rml.size(); ++i)
			if (_is_plain(rml, i))
				++count;
		return count;
	}

	// Returns the nth plain text character if n < _get_plain_length(rml), or '\0' otherwise.
	char _get_nth_plain(const std::string& rml, size_t n)
	{
		size_t count = 0;
		for (size_t i = 0; i < rml.size(); ++i) {
			if (_is_plain(rml, i)) {
				if (count == n) return rml[i];
				++count;
			}
		}
		return '\0';
	}

	// Replaces graphical plain text with non-breaking spaces, starting at offset.
	// This is used to prevent the text from jumping around when being typed out.
	std::string _replace_graphical_plain_with_nbsp(const std::string& rml, size_t offset)
	{
		std::string ret;
		size_t count = 0;
		for (size_t i = 0; i < rml.size(); ++i) {
			bool replace = false;
			if (_is_plain(rml, i)) {
				if (count >= offset && isgraph(rml[i]))
					replace = true;
				++count;
			}
			ret += replace ? "&nbsp;" : rml.substr(i, 1);
		}
		return ret;
	}

	void _on_textbox_keydown(int key)
	{
		if (!is_textbox_open())
			return;
		if (key == Rml::Input::KI_C)
			if (!is_textbox_typing())
				pop_textbox();
		if (key == Rml::Input::KI_X)
			if (is_textbox_typing())
				skip_textbox_typing();
			else
				pop_textbox();
	}

	void update_textbox(float dt)
	{
		Rml::ElementDocument* doc = _context->GetDocument("textbox");
		if (!doc) return;
		if (!_textbox_is_open) {
			doc->Hide();
			return;
		}
		doc->Show();
		_textbox_text = _textbox.text;
		_textbox_sprite = _textbox.sprite;
		_textbox_sprite_is_set = !_textbox.sprite.empty();
		size_t textbox_length = _get_plain_length(_textbox_text);
		if (_textbox_current_length >= textbox_length) {
			_textbox_current_length = textbox_length;
			_textbox_current_time = 0.f;
			return;
		}
		if (_textbox.typing_speed > 0.f) {
			float seconds_per_char = 1.f / _textbox.typing_speed;
			_textbox_current_time += dt;
			if (_textbox_current_time >= seconds_per_char) {
				_textbox_current_time -= seconds_per_char;
				if (isgraph(_get_nth_plain(_textbox_text, _textbox_current_length)))
					audio::play("event:/" + _textbox.typing_sound);
				++_textbox_current_length;
			}
		} else {
			_textbox_current_length = textbox_length;
		}
		_textbox_text = _replace_graphical_plain_with_nbsp(
			_textbox_text, _textbox_current_length);
	}

	bool is_textbox_open() {
		return _textbox_is_open;
	}

	bool is_textbox_typing() {
		return _textbox_current_length < _get_plain_length(_textbox_text);
	}

	void skip_textbox_typing() {
		_textbox_current_length = _get_plain_length(_textbox_text);
	}

	void open_textbox(const Textbox& textbox)
	{
		_textbox_is_open = true;
		_textbox = textbox;
		_textbox_current_time = 0.f;
		_textbox_current_length = 0;
		if (!_textbox.opening_sound.empty())
			audio::play("event:/" + _textbox.opening_sound);
		if (!textbox.next_textbox.empty())
			push_textbox_preset(textbox.next_textbox);
	}

	bool open_textbox_preset(const std::string& name)
	{
		Textbox textbox;
		if (!get_textbox_preset(name, textbox)) return false;
		open_textbox(textbox);
		return true;
	}

	void close_textbox()
	{
		_textbox_is_open = false;
		_textbox = Textbox();
	}

	void push_textbox(const Textbox& textbox) {
		_textbox_queue.push_back(textbox);
	}

	bool push_textbox_preset(const std::string& name)
	{
		Textbox textbox;
		if (!get_textbox_preset(name, textbox)) return false;
		push_textbox(textbox);
		return true;
	}

	bool pop_textbox()
	{
		if (_textbox_queue.empty()) {
			close_textbox();
			return false;
		}
		open_textbox(_textbox_queue.front());
		_textbox_queue.pop_front();
		return true;
	}
}