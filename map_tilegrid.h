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

	void recreate_tilegrid(const tiled::Map& map);

	sf::Vector2i get_tilegrid_size(); // in tiles
	sf::Vector2i get_tilegrid_tile_size(); // in pixels

	std::vector<sf::Vector2i> pathfind(const sf::Vector2i& start, const sf::Vector2i& end);
	std::vector<sf::Vector2f> pathfind(const sf::Vector2f& start, const sf::Vector2f& end);
}

