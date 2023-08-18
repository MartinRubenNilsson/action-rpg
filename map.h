#pragma once

namespace map
{
	// Loads all TMX maps in the assets/maps directory.
	// If a map is already open, it is closed first.
	void load_all();

	// Returns a list of names of all loaded maps.
	std::vector<std::string> get_list();

	// Returns the name of the currently open map.
	// If no map is open, returns an empty string.
	const std::string& get_name();

	// Returns an internal registry containing the deserialized version of the currently open map.
	entt::registry& get_registry();

	// Deserializes the map with the given name into the internal registry.
	// If a map is already open, it is closed first.
	bool open(const std::string& name);

	// Clears the internal registry.
	void close();
}
