#pragma once

namespace ecs
{
	// need to figure out a better place to put this...
	enum ITEM_TILE
	{
		ITEM_TILE_RUPEE = 40,
		ITEM_TILE_POTION = 77,
		ITEM_TILE_SPEAR = 96,
	};

	struct Animation
	{
		// Members ending in tile_id are indices into tiled::Tileset::tiles[],
		// while _frame_id is an index into tiled::Tile::animation[].

		Handle<tiled::Tileset> tileset;
		unsigned int tile_id = 0;
		unsigned int _previous_tile_id = 0;
		unsigned int _animated_tile_id = 0;
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
