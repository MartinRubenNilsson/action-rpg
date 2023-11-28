#include "tables.h"
#include "console.h"
#pragma warning(disable : 4996) // 'strncpy': This function or variable may be unsafe.
#pragma warning(disable : 4267)	// 'return': conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4244) // 'return': conversion from 'std::streamsize' to 'int', possible loss of data
#define CSV_IO_NO_THREAD
#include <fast-cpp-csv-parser/csv.h>

namespace tables
{
	template <unsigned column_count>
	using CSVReader = io::CSVReader<column_count, io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '\"'>>;

	std::vector<TextboxDataEntry> _textbox_table;

	void load_assets()
	{
		// Load textbox table
		try
		{
			CSVReader<6> table_csv("assets/tables/textbox.csv");
			table_csv.read_header(io::ignore_no_column,
				"name",
				"index",
				"sprite",
				"text",
				"speed",
				"sound");
			std::vector<TextboxDataEntry> table;
			TextboxDataEntry entry;
			while (table_csv.read_row(
				entry.name,
				entry.index,
				entry.sprite,
				entry.text,
				entry.speed,
				entry.sound))
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

	std::vector<TextboxDataEntry> query_textbox_data_entries(const std::string& name)
	{
		std::vector<TextboxDataEntry> entries;
		for (const TextboxDataEntry& entry : _textbox_table)
		{
			if (entry.name == name)
				entries.push_back(entry);
		}
		std::ranges::sort(entries, std::ranges::less{}, &TextboxDataEntry::index);
		return entries;
	}
}
