#include "data.h"
#include "console.h"
#pragma warning(disable : 4996) // 'strncpy': This function or variable may be unsafe.
#pragma warning(disable : 4267)	// 'return': conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4244) // 'return': conversion from 'std::streamsize' to 'int', possible loss of data
#include <fast-cpp-csv-parser/csv.h>

namespace data
{
	std::vector<Dialog> _dialogs;

	void load_dialogs()
	{
		try
		{
			io::CSVReader<2> dialogs_csv("assets/data/dialogs.csv");
			dialogs_csv.read_header(io::ignore_no_column, "id", "text");
			_dialogs.clear();
			Dialog dialog;
			while (dialogs_csv.read_row(dialog.id, dialog.text))
				_dialogs.push_back(dialog);
		}
		catch (const io::error::base& error)
		{
			console::log_error(error.what());
		}
	}

	const Dialog* get_dialog(const std::string& id)
	{
		for (const auto& dialog : _dialogs)
			if (dialog.id == id)
				return &dialog;
		return nullptr;
	}
}