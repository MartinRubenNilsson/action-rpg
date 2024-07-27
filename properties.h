#pragma once

class Properties
{
public:
	bool empty() const { return _properties.empty(); }
	void clear() { _properties.clear(); }
	bool contains(const std::string& name) const { return std::ranges::binary_search(_properties, name, {}, &Property::first); }
	void set_string(const std::string& name, const std::string& value) { _set(name, value); }
	bool get_string(const std::string& name, std::string& value) const { return _get(name, value); }
	void set_int(const std::string& name, int value) { _set(name, value); }
	bool get_int(const std::string& name, int& value) const { return _get(name, value); }
	void set_float(const std::string& name, float value) { _set(name, value); }
	bool get_float(const std::string& name, float& value) const { return _get(name, value); }
	void set_bool(const std::string& name, bool value) { _set(name, value); }
	bool get_bool(const std::string& name, bool& value) const { return _get(name, value); }
	void set_entity(const std::string& name, entt::entity value) { _set(name, value); }
	bool get_entity(const std::string& name, entt::entity& value) const { return _get(name, value); }
	void set_vector2f(const std::string& name, const Vector2f& value) { _set(name, value); }
	bool get_vector2f(const std::string& name, Vector2f& value) const { return _get(name, value); }

private:
	using Value = std::variant<
		std::string,
		int,
		float,
		bool,
		entt::entity,
		Vector2f
	>;

	using Property = std::pair<std::string, Value>;

	template <typename T> void _set(const std::string& name, const T& value);
	template <typename T> bool _get(const std::string& name, T& value) const;
	
	std::vector<Property> _properties; // sorted by Property::first
};

template<typename T>
inline void Properties::_set(const std::string& name, const T& value)
{
	// Do a binary search to find the property with the given name.
	auto it = std::ranges::lower_bound(_properties, name, {}, &Property::first);
	if (it != _properties.end() && it->first == name) {
		it->second = value;
	} else {
		_properties.emplace(it, name, value);
	}
}

template<typename T>
inline bool Properties::_get(const std::string& name, T& value) const
{
	// Do a binary search to find the property with the given name.
	auto it = std::ranges::lower_bound(_properties, name, {}, &Property::first);
	if (it == _properties.end()) return false;
	if (it->first != name) return false;
	if (!std::holds_alternative<T>(it->second)) return false;
	value = std::get<T>(it->second);
	return true;
}
