#include "stdafx.h"
#include "ui_textbox.h"
#include "ui_textbox_presets.h"
#include "audio.h"

namespace ui
{
	Rml::Input::KeyIdentifier _get_key_identifier(const Rml::Event& ev) {
		return (Rml::Input::KeyIdentifier)ev.GetParameter<int>("key_identifier", Rml::Input::KI_UNKNOWN);
	}

	struct TextboxEventListener : Rml::EventListener
	{
		void ProcessEvent(Rml::Event& ev) override
		{
			if (!is_textbox_visible()) return;
			switch (ev.GetId()) {
			case Rml::EventId::Keydown: {
				switch (_get_key_identifier(ev)) {
				case Rml::Input::KI_C: {
					if (!is_textbox_typing())
						pop_textbox();
				} break;
				case Rml::Input::KI_X: {
					if (is_textbox_typing())
						skip_textbox_typing();
					else
						pop_textbox();
				} break;
				}
			} break;
			}
		}
	};

	namespace bindings
	{
		std::string textbox_text; // RML
		bool textbox_has_sprite = false;
		std::string textbox_sprite;
		bool textbox_has_options = false;
		std::vector<std::string> textbox_options;
	}

	extern Rml::Context* _context;
	TextboxEventListener _textbox_event_listener;
	Textbox _textbox;
	std::deque<Textbox> _textbox_queue;
	float _textbox_typing_time = 0.f; // time since last character was typed
	size_t _textbox_typing_counter = 0; // number of characters typed

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
	size_t _get_plain_count(const std::string& rml)
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

	Rml::ElementDocument* _get_textbox_document() {
		return _context->GetDocument("textbox");
	}

	void _set_textbox_visible(bool visible)
	{
		if (Rml::ElementDocument* doc = _get_textbox_document())
			visible ? doc->Show() : doc->Hide();
	}

	void update_textbox(float dt)
	{
		if (!is_textbox_visible()) return;

		size_t plain_count = _get_plain_count(_textbox.text);
		if (_textbox_typing_counter < plain_count && _textbox.typing_speed > 0.f) {
			float seconds_per_char = 1.f / _textbox.typing_speed;
			_textbox_typing_time += dt;
			if (_textbox_typing_time >= seconds_per_char) {
				_textbox_typing_time -= seconds_per_char;
				if (isgraph(_get_nth_plain(_textbox.text, _textbox_typing_counter)))
					audio::play("event:/" + _textbox.typing_sound);
				++_textbox_typing_counter;
			}
		} else {
			_textbox_typing_counter = plain_count;
		}

		const bool finished_typing = (_textbox_typing_counter == plain_count);

		bindings::textbox_text = _replace_graphical_plain_with_nbsp(_textbox.text, _textbox_typing_counter);
		bindings::textbox_has_sprite = !_textbox.sprite.empty();
		bindings::textbox_sprite = _textbox.sprite;
		if (finished_typing) {
			bindings::textbox_has_options = !_textbox.options.empty();
			bindings::textbox_options = _textbox.options;
		} else {
			bindings::textbox_has_options = false;
			bindings::textbox_options.clear();
		}
	}

	void add_textbox_event_listeners()
	{
		if (Rml::ElementDocument* doc = _get_textbox_document()) {
			doc->AddEventListener(Rml::EventId::Keydown, &_textbox_event_listener);
		}
	}

	bool is_textbox_visible()
	{
		Rml::ElementDocument* doc = _get_textbox_document();
		return doc && doc->IsVisible();
	}

	bool is_textbox_typing() {
		return _textbox_typing_counter < _get_plain_count(_textbox.text);
	}

	void skip_textbox_typing() {
		_textbox_typing_counter = _get_plain_count(_textbox.text);
	}

	void open_textbox(const Textbox& textbox)
	{
		_set_textbox_visible(true);
		_textbox = textbox;
		_textbox_typing_time = 0.f;
		_textbox_typing_counter = 0;
		if (!_textbox.opening_sound.empty())
			audio::play("event:/" + _textbox.opening_sound);
	}

	void close_textbox()
	{
		_set_textbox_visible(false);
		_textbox = Textbox();
	}

	void close_all_textboxes()
	{
		close_textbox();
		_textbox_queue.clear();
	}

	void push_textbox(const Textbox& textbox) {
		_textbox_queue.push_back(textbox);
	}

	bool push_textbox_presets(const std::string& name)
	{
		std::vector<Textbox> textboxes;
		if (!get_textbox_presets(name, textboxes)) return false;
		for (const Textbox& textbox : textboxes)
			_textbox_queue.push_back(textbox);
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