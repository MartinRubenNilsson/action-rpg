#pragma once

#define ANIMATION_INVALID_TILE_ID UINT_MAX

namespace ecs
{
	Handle<tiled::Tileset> get_tileset(const std::string& name);

	struct Animation
	{
		// Members ending in tile_id are indices into tiled::Tileset::tiles[],
		// while _frame_id is an index into tiled::Tile::animation[].

		Handle<tiled::Tileset> tileset;
		unsigned int tile_id = ANIMATION_INVALID_TILE_ID;
		unsigned int _previous_tile_id = ANIMATION_INVALID_TILE_ID;
		unsigned int _animated_tile_id = ANIMATION_INVALID_TILE_ID;
		unsigned int _frame_id = 0;
		float progress = 0.f; // aka normalized time, in the range [0, 1]
		float speed = 1.f;
		bool loop = true;
		bool _dirty = false;
		bool _looped = false;
	};

	void update_animations(float dt);
	void update_animated_sprites(float dt);

	Animation& emplace_animation(entt::entity entity);
	Animation* get_animation(entt::entity entity);
	bool remove_animation(entt::entity entity);
}
