#include "stdafx.h"
#include "data.h"
#include <nlohmann/json.hpp>
#include "console.h"

namespace data
{
	nlohmann::json _json;

	void clear() {
		_json.clear();
	}

	bool load(const std::string& name)
	{
		std::string filename = "assets/saves/" + name + ".json";
		std::ifstream file(filename);
		if (!file.is_open())
		{
			console::log_error("Failed to open file: " + filename);
			return false;
		}
		nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
		if (json.is_discarded())
		{
			console::log_error("Failed to parse file: " + filename);
			return false;
		}
		_json = std::move(json);
		return true;
	}

	void save(const std::string& name)
	{
		std::ofstream file("assets/saves/" + name + ".json");
		file << _json.dump(4);
	}

	std::string dump() {
		return _json.dump(4);
	}

	void set_bool(const std::string& key, bool value) {
		_json[key] = value;
	}

	bool get_bool(const std::string& key, bool& value)
	{
		if (!_json.contains(key)) return false;
		if (!_json[key].is_boolean()) return false;
		value = _json[key];
		return true;
	}

	void set_int(const std::string& key, int value) {
		_json[key] = value;
	}

	bool get_int(const std::string& key, int& value)
	{
		if (!_json.contains(key)) return false;
		if (!_json[key].is_number_integer()) return false;
		value = _json[key];
		return true;
	}

	void set_float(const std::string& key, float value) {
		_json[key] = value;
	}

	bool get_float(const std::string& key, float& value) {
		if (!_json.contains(key)) return false;
		if (!_json[key].is_number_float()) return false;
		value = _json[key];
		return true;
	}

	void set_string(const std::string& key, const std::string& value) {
		_json[key] = value;
	}

	bool get_string(const std::string& key, std::string& value) {
		if (!_json.contains(key)) return false;
		if (!_json[key].is_string()) return false;
		value = _json[key];
		return true;
	}
}