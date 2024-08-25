#pragma once

namespace ecs
{
	Handle<tiled::Tileset> get_tileset(const std::string& name);
	Handle<graphics::Texture> get_tileset_texture(Handle<tiled::Tileset> tileset);

	struct TileAnimation
	{
		// Members ending in tile_id are indices into tiled::Tileset::tiles[],
		// while _frame_id is an index into tiled::Tile::animation[].

		Handle<tiled::Tileset> tileset;
		unsigned int tile_id = UINT_MAX;
		unsigned int _previous_tile_id = UINT_MAX;
		unsigned int _animated_tile_id = UINT_MAX;
		unsigned int _frame_id = 0;
		float progress = 0.f; // aka normalized time, in the range [0, 1]
		float speed = 1.f;
		bool loop = true;
		bool _dirty = false;
		bool _looped = false;
	};

	struct FlipbookAnimation
	{
		unsigned int rows = 0;
		unsigned int columns = 0;
		float fps = 0.f;
		float time = 0.f;
	};

	void update_tile_animations(float dt);
	void update_flipbook_animations(float dt);
	void update_animated_sprites(float dt);

	TileAnimation& emplace_tile_animation(entt::entity entity);
	TileAnimation* get_tile_animation(entt::entity entity);

	FlipbookAnimation& emplace_flipbook_animation(entt::entity entity);
}
