#pragma once

namespace tmx
{
	class Tileset;
}

namespace ecs
{
	struct Tile
	{
		tmx::Tileset* _tileset = nullptr; // For internal use only.
		uint32_t id = UINT32_MAX; // Global ID.
		float animation_time = 0.0f; // In seconds.
	};

	bool is_valid(const Tile& tile);
	bool is_animated(const Tile& tile);
	sf::IntRect get_texture_rect(const Tile& tile);
}
