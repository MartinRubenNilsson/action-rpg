#pragma once

namespace data
{
	// When loading and saving, data is read/written
	// to/from a JSON file in the assets/saves directory.
	// The name parameter is the name of the file, without
	// the .json extension.

	void clear();
	bool load(const std::string& name);
	void save(const std::string& name);
	std::string dump();

	void set_bool(const std::string& key, bool value);
	bool get_bool(const std::string& key, bool& value);
	void set_int(const std::string& key, int value);
	bool get_int(const std::string& key, int& value);
	void set_float(const std::string& key, float value);
	bool get_float(const std::string& key, float& value);
	void set_string(const std::string& key, const std::string& value);
	bool get_string(const std::string& key, std::string& value);
}
