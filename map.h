#pragma once

namespace map
{
	// Loads all TMX maps in the assets/tiled/maps directory.
	// This function should be called once at startup.
	void load_maps();

	// Returns a list of names of all loaded maps.
	std::vector<std::string> get_map_names();

	// Reloads all textures referenced by any map that
	// has been opened at least once.
	void reload_textures();

	// Requests that the map with the given name be opened.
	// The map will not be opened until update() is called.
	// If the map is already open, it will not be re-opened unless force is true.
	// Returns true if the map was found, false otherwise.
	bool open(const std::string& map_name, bool force = false);

	// Requests that the current map be closed.
	// The map will not be closed until update() is called.
	void close();

	// Requests that the current map be reset.
	// The map will not be reset until update() is called.
	void reset();

	// Executes the requests made by open(), close(), and reset().
	// This function should be called once per frame.
	void update();

	// Returns the name of the current map.
	// If no map is open, returns an empty string.
	std::string get_name();

	// Returns the bounds of the current map in world space units (meters).
	// If no map is open, returns an empty rectangle.
	sf::FloatRect get_bounds();

	// Spawns an entity from the given Tiled object template,
	// which must be used at least once in the current map.
	// Returns entt::null if the template was not found.
	entt::entity spawn(
		const std::string& template_name,
		const sf::Vector2f& position, // In world space units (meters).
		float depth = 100.f);

	// Sets the name of the entity that should be used as
	// the player's starting position when a map is opened.
	// You may call this function before or after open(),
	// as long as you call it before update().
	void set_spawnpoint(const std::string& entity_name);
}
