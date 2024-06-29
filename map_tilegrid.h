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
	sf::Vector2i world_to_tile(const Vector2f& world_pos);
	Vector2f get_tile_center(const sf::Vector2i& tile);
	TerrainType get_terrain_type_at(const Vector2f& world_pos);
	bool pathfind(const sf::Vector2i& start, const sf::Vector2i& end, std::vector<sf::Vector2i>& path);
}

