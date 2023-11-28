#pragma once

namespace tables
{
	struct TextboxDataEntry
	{
		std::string name;
		int index = 0;
		std::string sprite;
		std::string text; // RML markdown
		float speed = 0.f; // chars per second, 0 = instant
		std::string sound;
	};

	// Loads certain CSV tables in the assets/tables directory.
	void load_assets();

	std::vector<TextboxDataEntry> query_textbox_data_entries(const std::string& name);
}

