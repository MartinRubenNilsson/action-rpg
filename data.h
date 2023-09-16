#pragma once

namespace data
{
	struct TextboxEntry
	{
		std::string name;
		int index = 0;
		std::string sprite;
		std::string text; // RML markdown
		float speed = 0.f; // chars per second, 0 = instant
		std::string sound;
	};

	void load_textbox_table();

	std::vector<TextboxEntry> get_textbox_entries(const std::string& name);
}

