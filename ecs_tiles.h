#pragma once
#include "tiled.h"

namespace ecs
{
	class Tile
	{
	public:
		const tiled::Tile* tile = nullptr;
		std::string action = "idle";
		char direction = 'd';
		sf::Sprite sprite;
		bool visible = true;
		float depth = 0.f; // Tiles with lower depth values are drawn first.
		float animation_time = 0.0f; // In seconds.
		float animation_speed = 1.0f; // 1 is normal speed, 2 is double speed, etc.
	};

	void update_tiles(float dt);
}
