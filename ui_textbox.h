#pragma once

namespace ui
{
	struct TextboxEntry
	{
		std::string text; // RML markdown
		std::string sprite;

		bool empty() const;
	};

	void update_textbox(float dt);

	bool is_textbox_visible();
	bool is_textbox_typing();
	void skip_textbox_typing();
	void push_textbox_entry(const TextboxEntry& entry);
	void pop_textbox_entry();
	void set_textbox_text(const std::string& text);
	void set_textbox_sprite(const std::string& sprite);
}
