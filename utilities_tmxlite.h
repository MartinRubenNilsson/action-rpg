#pragma once
#include <tmxlite/Tileset.hpp>

namespace tmx
{
	struct AnimatedTile
	{
		Tileset* tileset = nullptr;
		uint32_t tile_id = UINT32_MAX; // The global tile ID of the first frame.
		float time = 0.0f; // In seconds.
	};

	sf::IntRect get_texture_rect(const Tileset& tileset, uint32_t tile_id);

	float get_total_duration(const AnimatedTile& animated_tile);
	uint32_t get_current_tile_id(const AnimatedTile& animated_tile);
	sf::IntRect get_current_texture_rect(const AnimatedTile& animated_tile);
}
