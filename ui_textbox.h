#pragma once

namespace ui
{
	extern const std::string TEXTBOX_SPRITE_SKULL;
	extern const std::string TEXTBOX_DEFAULT_TYPING_SOUND;
	extern const float TEXTBOX_DEFAULT_TYPING_SPEED;

	struct Textbox
	{
		std::string text; // RML
		std::string sprite;
		std::string typing_sound = TEXTBOX_DEFAULT_TYPING_SOUND;
		float typing_speed = TEXTBOX_DEFAULT_TYPING_SPEED; // in CPS, 0 = instant
	};

	void update_textbox(float dt);

	bool is_textbox_open();
	bool is_textbox_typing();
	void skip_textbox_typing();

	// Opens the given textbox immediately, skipping the queue.
	void open_textbox(const Textbox& textbox);
	// Closes the current textbox, but leaves the queue as-is.
	void close_textbox();
	// Pushes the given textbox to the queue. Call pop_textbox() to open it.
	void push_textbox(const Textbox& textbox);
	// If the queue is nonempty, pops the next textbox, opens it, and returns true.
	// Otherwise, closes the current textbox and returns false.
	bool pop_textbox();
}
