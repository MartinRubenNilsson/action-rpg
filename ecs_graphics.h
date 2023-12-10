#pragma once
#include "timer.h"

namespace tiled
{
	struct Tile;
}

namespace ecs
{
	struct Sprite
	{
		sf::Sprite sprite;
		float z = 0.f; // Higher z-values are drawn on top of lower z-values.
		bool visible = true;
	};

	class Animation
	{
	public:
		Animation(const tiled::Tile* tile);

		float speed = 1.0f;
		bool loop = true;
		
		// Returns true if the frame changed.
		bool update(float dt);
		bool play(const std::string& tile_class);
		bool is_playing(const std::string& tile_class) const;
		const tiled::Tile* get_tile() const { return _tile; }
		const tiled::Tile* get_frame() const { return _frame; }
		const Timer& get_timer() const { return _timer; }

	private:
		const tiled::Tile* _tile = nullptr; // currently playing animated tile
		const tiled::Tile* _frame = nullptr; // current frame in _tile->animation
		Timer _timer;
	};

	struct SlimeAnimationController {};

	void update_graphics(float dt);

	void emplace_sprite(entt::entity entity, const Sprite& sprite);
	void emplace_animation(entt::entity entity, const Animation& anim);
	void emplace_slime_animation_controller(entt::entity entity);
}
