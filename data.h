#pragma once

namespace data
{
	struct Text
	{
		std::string id;
		std::string content;
	};

	void load_texts();

	// Returns nullptr if the text is not found.
	const Text* get_text(const std::string& id);
}

