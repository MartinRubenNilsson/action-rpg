#pragma once

namespace tiled
{
	struct Map;
}

namespace map
{
	enum class TerrainType
	{
		None,
		Dirt,
		LightGrass,
		DarkGrass,
		Cobblestone,
		ShallowWater,
		DeepWater,
	};

	std::string to_string(TerrainType type);

	void create_tilegrid(const tiled::Map& map);
	void destroy_tilegrid();

	sf::Vector2i get_grid_size(); // in tiles
	sf::Vector2i get_tile_size(); // in pixels
	sf::Vector2i world_to_tilegrid(const sf::Vector2f& world_pos);
	sf::Vector2f tilegrid_to_world(const sf::Vector2i& tile_pos); // center of tile
	TerrainType get_terrain_type_at(const sf::Vector2f& world_pos);
	bool pathfind(const sf::Vector2i& start, const sf::Vector2i& end, std::vector<sf::Vector2i>& path);
}

