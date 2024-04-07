#include "stdafx.h"
#include "ui_textbox.h"
#include "audio.h"

namespace ui
{
	Rml::Input::KeyIdentifier _get_key_identifier(const Rml::Event& ev) {
		return (Rml::Input::KeyIdentifier)ev.GetParameter<int>("key_identifier", Rml::Input::KI_UNKNOWN);
	}

	void _on_textbox_keydown_c();
	void _on_textbox_keydown_up();
	void _on_textbox_keydown_down();

	struct TextboxEventListener : Rml::EventListener
	{
		void ProcessEvent(Rml::Event& ev) override
		{
			if (!is_textbox_open()) return;
			switch (ev.GetId()) {
			case Rml::EventId::Keydown: {
				switch (_get_key_identifier(ev)) {
				case Rml::Input::KI_C:
					_on_textbox_keydown_c();
					break;
				case Rml::Input::KI_UP:
					_on_textbox_keydown_up();
					break;
				case Rml::Input::KI_DOWN:
					_on_textbox_keydown_down();
					break;
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
		size_t textbox_selected_option = 0;

		void _clear_textbox_bindings()
		{
			textbox_text.clear();
			textbox_has_sprite = false;
			textbox_sprite.clear();
			textbox_has_options = false;
			textbox_options.clear();
			textbox_selected_option = 0;
		}
	}

	extern Rml::Context* _context;
	TextboxEventListener _textbox_event_listener;
	std::optional<Textbox> _textbox;
	std::deque<Textbox> _textbox_queue;
	float _textbox_typing_time = 0.f; // time since last character was typed
	size_t _textbox_typing_counter = 0; // number of characters typed

	void _on_textbox_keydown_c()
	{
		if (!_textbox) return;
		if (is_textbox_typing()) {
			skip_textbox_typing();
		} else if (_textbox->options_callback &&
			bindings::textbox_selected_option < bindings::textbox_options.size()) {
			const std::string& option = bindings::textbox_options[bindings::textbox_selected_option];
			_textbox->options_callback(option);
			audio::play("event:/ui/snd_button_click");
		} else {
			open_next_textbox_in_queue();
		}
	}

	void _on_textbox_keydown_up()
	{
		if (bindings::textbox_selected_option > 0) {
			bindings::textbox_selected_option--;
			audio::play("event:/ui/snd_button_hover");
		}
	}

	void _on_textbox_keydown_down()
	{
		if (bindings::textbox_selected_option + 1 < bindings::textbox_options.size()) {
			bindings::textbox_selected_option++;
			audio::play("event:/ui/snd_button_hover");
		}
	}

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

	void _set_textbox_document_visible(bool visible)
	{
		if (Rml::ElementDocument* doc = _get_textbox_document())
			visible ? doc->Show() : doc->Hide();
	}

	void update_textbox(float dt)
	{
		if (_textbox) {
			_set_textbox_document_visible(true);
		} else {
			_set_textbox_document_visible(false);
			return;
		}

		size_t plain_count = _get_plain_count(_textbox->text);
		if (_textbox_typing_counter < plain_count && _textbox->typing_speed > 0.f) {
			float seconds_per_char = 1.f / _textbox->typing_speed;
			_textbox_typing_time += dt;
			if (_textbox_typing_time >= seconds_per_char) {
				_textbox_typing_time -= seconds_per_char;
				if (isgraph(_get_nth_plain(_textbox->text, _textbox_typing_counter)))
					audio::play("event:/" + _textbox->typing_sound);
				++_textbox_typing_counter;
			}
		} else {
			_textbox_typing_counter = plain_count;
		}

		const bool finished_typing = (_textbox_typing_counter == plain_count);

		bindings::textbox_text = _replace_graphical_plain_with_nbsp(
			_textbox->text, _textbox_typing_counter);
		bindings::textbox_has_sprite = !_textbox->sprite.empty();
		bindings::textbox_sprite = _textbox->sprite;
		if (finished_typing) {
			bindings::textbox_has_options = !_textbox->options.empty();
			bindings::textbox_options = _textbox->options;
		} else {
			bindings::textbox_has_options = false;
			bindings::textbox_options.clear();
			bindings::textbox_selected_option = 0;
		}
	}

	void add_textbox_event_listeners()
	{
		if (Rml::ElementDocument* doc = _get_textbox_document()) {
			doc->AddEventListener(Rml::EventId::Keydown, &_textbox_event_listener);
		}
	}

	bool is_textbox_open() {
		return _textbox.has_value();
	}

	bool is_textbox_typing()
	{
		if (!_textbox) return false;
		return _textbox_typing_counter < _get_plain_count(_textbox->text);
	}

	void skip_textbox_typing()
	{
		if (!_textbox) return;
		_textbox_typing_counter = _get_plain_count(_textbox->text);
	}

	void open_textbox(const Textbox& textbox)
	{
		_textbox = textbox;
		_textbox_typing_time = 0.f;
		_textbox_typing_counter = 0;
		if (!_textbox->opening_sound.empty())
			audio::play("event:/" + _textbox->opening_sound);
	}

	void enqueue_textbox(const Textbox& textbox) {
		_textbox_queue.push_back(textbox);
	}

	void open_or_enqueue_textbox(const Textbox& textbox)
	{
		if (is_textbox_open()) {
			enqueue_textbox(textbox);
		} else {
			open_textbox(textbox);
		}
	}

	bool open_next_textbox_in_queue()
	{
		if (_textbox_queue.empty()) {
			close_textbox();
			return false;
		}
		open_textbox(_textbox_queue.front());
		_textbox_queue.pop_front();
		return true;
	}

	void close_textbox()
	{
		_textbox.reset();
		bindings::_clear_textbox_bindings();
	}

	void close_textbox_and_clear_queue()
	{
		close_textbox();
		_textbox_queue.clear();
	}

	void open_or_enqueue_textbox_presets(const std::string& id)
	{
		for (const Textbox& textbox : find_textbox_presets(id))
			open_or_enqueue_textbox(textbox);
	}
}