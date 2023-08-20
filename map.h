#pragma once

namespace map
{
	void initialize();
	void shutdown();

	// Loads all tilesets in the assets/tilesets directory.
	// This function should only be called once.
	void load_tilesets();

	// (Re)loads all TMX maps in the assets/maps directory.
	// If a map is already open, it is closed first.
	void load_maps();

	// Returns a list of names of all loaded maps.
	std::vector<std::string> get_list();

	// Returns the name of the currently open map.
	// If no map is open, returns an empty string.
	const std::string& get_name();

	// Returns the bounds of the currently open map.
	// If no map is open, returns an empty rectangle.
	sf::FloatRect get_bounds();

	// Returns the size of a tile in the currently open map.
	// If no map is open, returns (0, 0).
	const sf::Vector2u& get_tile_size();

	// Returns a reference to the physics world of the currently open map.
	// If no map is open, the returned world is empty.
	b2World& get_world();

	// Returns a reference to the ECS registry of the currently open map.
	// If no map is open, the returned registry is empty.
	entt::registry& get_registry();

	// Opens the map with the given name.
	// If a map is already open, it is closed first.
	bool open(const std::string& name);

	// Closes the currently open map.
	void close();
}
