#pragma once

namespace tiled
{
	struct Tile;
	struct Tileset;
}

namespace ecs
{
	struct Sprite
	{
		sf::Sprite sprite;
		float depth = 0.f; // Sprites with lower depth values are drawn first.
		bool visible = true;
	};

	enum class AnimationType
	{
		DEFAULT,
		PLAYER,
	};

	struct Animation
	{
		AnimationType type = AnimationType::DEFAULT;
		float time = 0.0f; // in seconds
		float speed = 1.0f;
		std::string state = "idle";
		char direction = 'd';
		std::string tile_class;
		const tiled::Tile* _current_tile = nullptr; // read-only
	};

	void update_graphics(float dt);
}
