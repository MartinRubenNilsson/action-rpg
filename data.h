#pragma once

namespace data
{
	struct TextboxEntry
	{
		std::string name;
		int index = 0;
		std::string sprite;
		std::string text; // RML markdown
		float typing_speed = 0.f; // chars per second, 0 = instant
	};

	void load_textbox_table();

	std::vector<TextboxEntry> get_textbox_entries(const std::string& name);
}

