#include "data.h"
#pragma warning(disable : 4996) // 'strncpy': This function or variable may be unsafe.
#pragma warning(disable : 4267)	// 'return': conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4244) // 'return': conversion from 'std::streamsize' to 'int', possible loss of data
#define CSV_IO_NO_THREAD
#include <fast-cpp-csv-parser/csv.h>
#include "console.h"

namespace data
{
	template <unsigned column_count>
	using CSVReader = io::CSVReader<column_count,
		io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '\"'>>;

	std::vector<TextboxEntry> _textbox_table;

	void load_textbox_table()
	{
		try
		{
			CSVReader<5> table_csv("assets/data/textbox.csv");
			table_csv.read_header(io::ignore_no_column,
				"name",
				"index",
				"sprite",
				"text",
				"typing_speed");
			std::vector<TextboxEntry> table;
			TextboxEntry entry;
			while (table_csv.read_row(
				entry.name,
				entry.index,
				entry.sprite,
				entry.text,
				entry.typing_speed))
			{
				table.push_back(entry);
			}
			_textbox_table = std::move(table);
		}
		catch (const io::error::base& error)
		{
			console::log_error(error.what());
		}
	}

	std::vector<TextboxEntry> get_textbox_entries(const std::string& name)
	{
		std::vector<TextboxEntry> entries;
		for (const auto& entry : _textbox_table)
		{
			if (entry.name == name)
				entries.push_back(entry);
		}
		std::ranges::sort(entries, std::ranges::less{}, &TextboxEntry::index);
		return entries;
	}
}
