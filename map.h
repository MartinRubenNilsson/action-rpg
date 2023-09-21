#pragma once

namespace map
{
	// Reloads all textures referenced by any map that
	// has been opened at least once.
	void reload_textures();

	// Loads all TMX maps in the assets/maps directory.
	// This function should only be called once.
	void load_maps();

	// Returns a list of names of all loaded maps.
	std::vector<std::string> get_loaded_maps();

	// Requests that the map with the given name be opened.
	// The map will not be opened until update() is called.
	// Returns true if the map was found, false otherwise.
	bool open(const std::string& map_name);

	// Requests that the current map be closed.
	// The map will not be closed until update() is called.
	void close();

	// Executes the open/close requests made by open() and close().
	void update();

	// Returns the name of the current map.
	// If no map is open, returns an empty string.
	std::string get_name();

	// Returns the bounds of the current map.
	// If no map is open, returns an empty rectangle.
	sf::FloatRect get_bounds();
}
