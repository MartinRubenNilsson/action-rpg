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
		float z = 0.f; // Higher z-values are drawn on top of lower z-values.
		bool visible = true;
	};

	enum class AnimationType
	{
		Default,
		Player,
	};

	struct Animation
	{
		AnimationType type = AnimationType::Default;
		float time = 0.0f; // in seconds
		float speed = 1.0f;
		std::string state = "idle";
		char direction = 'd';
		std::string tile_class;
		const tiled::Tile* _current_tile = nullptr; // read-only
	};

	void update_graphics(float dt);

	void emplace_sprite(entt::entity entity, const Sprite& sprite);
	void emplace_animation(entt::entity entity, const Animation& anim);
}
