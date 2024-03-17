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
	sf::Vector2i world_to_tile(const sf::Vector2f& world_pos);
	sf::Vector2f get_tile_center(const sf::Vector2i& tile_pos); // in pixels
	TerrainType get_terrain_type_at(const sf::Vector2f& world_pos);
	bool pathfind(const sf::Vector2i& start_tile_pos, const sf::Vector2i& end_tile_pos, std::vector<sf::Vector2i>& path);
}

