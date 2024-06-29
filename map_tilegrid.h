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

	Vector2i get_grid_size(); // in tiles
	Vector2i get_tile_size(); // in pixels
	Vector2i world_to_tile(const Vector2f& world_pos);
	Vector2f get_tile_center(const Vector2i& tile);
	TerrainType get_terrain_type_at(const Vector2f& world_pos);
	bool pathfind(const Vector2i& start, const Vector2i& end, std::vector<Vector2i>& path);
}

