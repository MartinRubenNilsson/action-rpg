#pragma once
#include "ui_textbox_presets.h"

namespace ui
{
	void update_textbox(float dt);

	bool is_textbox_open();
	bool is_textbox_typing();
	void skip_textbox_typing();

	// Opens the given textbox. If textbox.next_textbox is a
	// valid textbox preset, it will be pushed to the queue.
	void open_textbox(const Textbox& textbox);
	// Opens the given textbox preset, if it exists.
	bool open_textbox_preset(const std::string& name);
	// Closes the current textbox. The queue is not affected.
	void close_textbox();
	// Closes the current textbox and clears the queue.
	void close_all_textboxes();
	// Pushes the given textbox to the queue. Call pop_textbox() to open it.
	void push_textbox(const Textbox& textbox);
	// Pushes the given textbox preset to the queue, if it exists.
	bool push_textbox_preset(const std::string& name);
	// If the queue is nonempty, pops the next textbox, opens it, and returns true.
	// Otherwise, closes any currenty open textbox and returns false.
	bool pop_textbox();
}
