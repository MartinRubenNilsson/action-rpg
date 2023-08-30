#pragma once

namespace data
{
	struct Dialog
	{
		std::string id;
		std::string text;
	};

	void load_dialogs();

	// Returns nullptr if the dialog is not found.
	const Dialog* get_dialog(const std::string& id);
}

