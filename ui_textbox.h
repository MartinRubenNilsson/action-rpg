#pragma once

namespace ui
{
	struct Textbox
	{
		static const std::string SPRITE_SKULL;
		static const std::string SPRITE_GOLDEN_KEY;
		static const std::string OPENING_SOUND_ITEM_FANFARE;
		static const std::string DEFAULT_TYPING_SOUND;
		static const float DEFAULT_TYPING_SPEED;

		std::string id;
		std::string text; // RML
		std::string sprite;
		std::string opening_sound; // name of sound event
		std::string typing_sound = DEFAULT_TYPING_SOUND;
		float typing_speed = DEFAULT_TYPING_SPEED; // in chars per second, 0 = instant
		std::vector<std::string> options;
		void (*options_callback)(const std::string& option) = nullptr;
	};

	namespace bindings
	{
		extern std::string textbox_text; // RML
		extern bool textbox_has_sprite;
		extern std::string textbox_sprite;
		extern bool textbox_has_options;
		extern std::vector<std::string> textbox_options;
		extern size_t textbox_selected_option;
	}

	void add_textbox_event_listeners();
	void register_textbox_presets();
	void update_textbox(float dt);

	bool is_textbox_open();
	bool is_textbox_typing();
	void skip_textbox_typing();

	// In addition to the current textbox (which may or may not be closed/empty), we store a queue of textboxes.
	// This is useful for sequencing textboxes, e.g. for a conversation. The interface works as follows:
	// 
	// - open_textbox(): immediately opens a new textbox, closing the current one if necessary.
	// - enqueue_textbox(): appends a textbox to the queue without affecting the current textbox.
	// - open_or_enqueue_textbox(): opens the textbox immediately if there is no current textbox, otherwise enqueues it.
	// - open_next_textbox_in_queue(): opens the next textbox in the queue, if any.
	// - close_textbox(): closes the current textbox without affecting the queue.
	// - close_textbox_and_clear_queue(): closes the current textbox and clears the queue.

	void open_textbox(const Textbox& textbox);
	void enqueue_textbox(const Textbox& textbox);
	void open_or_enqueue_textbox(const Textbox& textbox);
	bool open_next_textbox_in_queue();
	void close_textbox();
	void close_textbox_and_clear_queue();

	std::span<const Textbox> find_textbox_presets(const std::string& id);
	void open_or_enqueue_textbox_presets(const std::string& id);
}
