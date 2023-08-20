#pragma once

namespace map
{
	// Window must be kept alive until shutdown() is called.
	void initialize(sf::RenderWindow& window);
	void shutdown();

	// Loads all tilesets in the assets/tilesets directory.
	// This function should only be called once.
	void load_tilesets();

	// Loads all TMX maps in the assets/maps directory.
	// This function should only be called once.
	void load_maps();

	// Returns a list of names of all loaded maps.
	std::vector<std::string> get_list();

	// Returns the name of the current map.
	// If no map is open, returns an empty string.
	std::string get_name();

	// Returns the bounds of the current map.
	// If no map is open, returns an empty rectangle.
	sf::FloatRect get_bounds();

	// Returns the size of a tile in the current map.
	// If no map is open, returns (0, 0).
	sf::Vector2u get_tile_size();

	// Returns a reference to the physics world of the current map.
	// If no map is open, the returned world is empty.
	b2World& get_world();

	// Returns a reference to the ECS registry of the current map.
	// If no map is open, the returned registry is empty.
	entt::registry& get_registry();

	// Opens the map with the given name.
	// If a map is already open, it is closed first.
	bool open(const std::string& name);

	// Closes the current map.
	void close();
}
