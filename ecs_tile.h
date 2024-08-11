#pragma once
#include "timer.h"
#include "properties.h"

namespace tiled
{
	struct Tile;
}

namespace graphics
{
	extern Handle<Shader> sprite_shader;
}

namespace map
{
	size_t get_object_layer_index();
}

namespace ecs
{
	enum TILE_FLAGS : unsigned int
	{
		TILE_VISIBLE        = (1 << 0), // if not set, the tile is not rendered
		TILE_FLIP_X         = (1 << 1), // flip the tile horizontally
		TILE_FLIP_Y         = (1 << 2), // flip the tile vertically
		TILE_FLIP_DIAGONAL  = (1 << 3), // flip the bottom-left and top-right corners
		TILE_LOOP           = (1 << 4), // loop the animation
		TILE_FLIP_X_ON_LOOP = (1 << 5), // flip the tile horizontally when the animation loops
		TILE_FRAME_CHANGED  = (1 << 6), // the animation frame changed last update
		TILE_LOOPED	        = (1 << 7), // the animation looped last update
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
		unsigned int tile_id = 0; // index into tiled::Tileset::tiles
		Handle<graphics::Shader> shader = graphics::sprite_shader;
		Handle<graphics::Texture> texture;
		Vector2f position; // in pixels
		Vector2f pivot; // in pixels, relative to the top-left corner
		Vector2f sorting_pivot; // in pixels, relative to the top-left corner
		uint8_t sorting_layer = (uint8_t)map::get_object_layer_index();
		Color color = colors::WHITE;
		Timer animation_timer;
		float animation_speed = 1.f;
		unsigned int animation_frame = 0; // index into tiled::Tile::animation
		unsigned int flags = TILE_VISIBLE | TILE_LOOP;

		bool set_tile(const tiled::Tile* tile);
		bool set_tile(unsigned int id); // uses the current tileset
		bool set_tileset(const std::string& tileset_name);
		bool set_tile(unsigned int x, unsigned int y); // uses the current tileset
		void get_texture_rect(unsigned int& left, unsigned int& top, unsigned int& width, unsigned int& height) const;
		unsigned int get_id(bool account_for_animation = true) const;
		Vector2u get_coords(bool account_for_animation = true) const;
		bool has_animation() const;
		void set_flag(unsigned int flag, bool value);
		bool get_flag(unsigned int flag) const;
		void set_rotation(int clockwise_quarter_turns);

		const tiled::Tile* _get_tile(bool account_for_animation) const;
	};

	void update_tiles(float dt);
	void add_tile_sprites_for_drawing(const Vector2f& camera_min, const Vector2f& camera_max);

	Tile& emplace_tile(entt::entity entity);
	Tile* get_tile(entt::entity entity);
	bool remove_tile(entt::entity entity);
}
