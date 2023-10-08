#include "stdafx.h"
#include "data.h"
#include <nlohmann/json.hpp>

namespace data
{
	nlohmann::json _json;

	bool load(const std::string& name)
	{
		return false;
	}

	void save(const std::string& name)
	{
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