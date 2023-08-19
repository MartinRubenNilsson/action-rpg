#pragma once

namespace map
{
	// (Re)loads all TMX maps in the assets/maps directory.
	// If a map is already open, it is closed first.
	void load_all();

	// Returns a list of names of all loaded maps.
	std::vector<std::string> get_list();

	// Returns the name of the currently open map.
	// If no map is open, returns an empty string.
	const std::string& get_name();

	// Returns a reference to the registry of the currently open map.
	// If no map is open, the returned registry is empty.
	entt::registry& get_registry();

	// Returns the bounds of the currently open map.
	// If no map is open, returns an empty rectangle.
	sf::FloatRect get_bounds();

	// Opens the map with the given name.
	// If a map is already open, it is closed first.
	bool open(const std::string& name);

	// Closes the currently open map.
	void close();
}
