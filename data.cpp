#include "data.h"
#include "console.h"
#pragma warning(disable : 4996) // 'strncpy': This function or variable may be unsafe.
#pragma warning(disable : 4267)	// 'return': conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4244) // 'return': conversion from 'std::streamsize' to 'int', possible loss of data
#include <fast-cpp-csv-parser/csv.h>

namespace data
{
	std::vector<Text> _texts;

	void load_texts()
	{
		try
		{
			io::CSVReader<2> texts_csv("assets/data/texts.csv");
			texts_csv.read_header(io::ignore_no_column, "id", "english");
			_texts.clear();
			Text text;
			while (texts_csv.read_row(text.id, text.content))
				_texts.push_back(text);
		}
		catch (const io::error::base& error)
		{
			console::log_error(error.what());
		}
	}

	const Text* get_text(const std::string& id)
	{
		for (const auto& dialog : _texts)
			if (dialog.id == id)
				return &dialog;
		return nullptr;
	}
}