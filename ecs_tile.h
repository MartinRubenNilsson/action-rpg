#pragma once

namespace ecs
{
	enum TILE_FLAGS : unsigned int
	{
		TILE_FLIP_X_ON_LOOP = (1 << 4), // flip the tile horizontally when the animation loops
	};

	// need to figure out a better place to put this...
	enum ITEM_TILE
	{
		ITEM_TILE_RUPEE = 40,
		ITEM_TILE_POTION = 77,
		ITEM_TILE_SPEAR = 96,
	};

	struct Tile
	{
		Handle<tiled::Tileset> tileset;
		unsigned int id = 0; // index into tiled::Tileset::tiles[]
		unsigned int tex_rect_l = 0; // in pixels
		unsigned int tex_rect_t = 0; // in pixels
		unsigned int tex_rect_r = 0; // in pixels
		unsigned int tex_rect_b = 0; // in pixels

		Vector2f position; // in pixels
		Vector2f pivot; // in pixels, relative to the top-left corner
		Vector2f sorting_pivot; // in pixels, relative to the top-left corner
		unsigned int flags = 0;

		void set_texture_rect(unsigned int id);
		bool set_tileset(Handle<tiled::Tileset> handle);
		bool set_tileset(const std::string& tileset_name);
		bool set_tile(unsigned int id);
		void set_flag(unsigned int flag, bool value);
		bool get_flag(unsigned int flag) const;
		//void set_rotation(int clockwise_quarter_turns);
	};

	struct TileAnimation
	{
		float progress = 0.f; // aka normalized time, in the range [0, 1]
		float speed = 1.f;
		unsigned int frame = 0; // index into tiled::Tile::animation[]
		bool loop = true;
		bool frame_changed = false;
	};

	void update_tile_positions(float dt);
	void update_tile_animations(float dt);
	void update_tile_sprites(float dt);

	Tile& emplace_tile(entt::entity entity);
	Tile* get_tile(entt::entity entity);
	bool remove_tile(entt::entity entity);

	TileAnimation& emplace_tile_animation(entt::entity entity);
}
