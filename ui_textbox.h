#pragma once

namespace ui
{
	struct Textbox;

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
	void update_textbox(float dt);

	bool is_textbox_visible();
	bool is_textbox_typing();
	void skip_textbox_typing();

	// Opens the given textbox.
	void open_textbox(const Textbox& textbox);
	// Closes the current textbox. The queue is not affected.
	void close_textbox();
	// Closes the current textbox and clears the queue.
	void close_all_textboxes();
	// Pushes the given textbox to the queue. Call pop_textbox() to open it.
	void push_textbox(const Textbox& textbox);
	// Pushes the given textbox presets to the queue, if any exist.
	bool push_textbox_presets(const std::string& name);
	// If the queue is nonempty, pops the next textbox, opens it, and returns true.
	// Otherwise, closes any currenty open textbox and returns false.
	bool pop_textbox();
}
