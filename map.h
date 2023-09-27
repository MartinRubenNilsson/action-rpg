#pragma once

namespace map
{
	// Executes the open/close requests made by open(), reopen(), and close().
	void update();

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
	// If the map is already open, it will not be re-opened unless force_open is true.
	// Returns true if the map was found, false otherwise.
	bool open(const std::string& map_name, bool force_open = false);

	// Requests that the current map be re-opened.
	// Does nothing if no map is currently open.
	void reopen();

	// Requests that the current map be closed.
	// The map will not be closed until update() is called.
	void close();

	// Returns the name of the current map.
	// If no map is open, returns an empty string.
	std::string get_name();

	// Returns the bounds of the current map in pixels.
	// If no map is open, returns an empty rectangle.
	sf::FloatRect get_bounds();

	// Sets the name of the entity that should be used as
	// the player's starting position when a map is opened.
	// You may call this function before or after open(),
	// as long as you call it before update().
	void set_spawnpoint(const std::string& entity_name);
}
