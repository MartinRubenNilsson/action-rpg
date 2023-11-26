#pragma once

namespace ui
{
	void update_textbox(float dt);

	bool is_textbox_visible();
	bool is_textbox_typing();
	void skip_textbox_typing();

	// Advances the textbox to the next entry in the queue.
	// If the queue is empty, the textbox will be hidden.
	void advance_textbox_entry();

	// Adds the given entries to the end of the queue.
	// If the queue was empty, the textbox will be shown
	// and the first entry in the queue will be displayed.
	void add_textbox_entries(const std::string& name);

	// Sets the queue to only contain the given entries.
	// If the queue was empty, the textbox will be shown
	// and the first entry in the queue will be displayed.
	void set_textbox_entries(const std::string& name);
}
