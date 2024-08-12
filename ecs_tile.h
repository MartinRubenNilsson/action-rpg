#pragma once

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
		TILE_FLIP_X_ON_LOOP = (1 << 4), // flip the tile horizontally when the animation loops
		TILE_FRAME_CHANGED  = (1 << 5), // the animation frame changed last update
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
		unsigned int texture_rect_left = 0; // in pixels
		unsigned int texture_rect_top = 0; // in pixels
		unsigned int texture_rect_width = 0; // in pixels
		unsigned int texture_rect_height = 0; // in pixels

		Handle<graphics::Shader> shader = graphics::sprite_shader;
		Handle<graphics::Texture> texture;
		Vector2f position; // in pixels
		Vector2f pivot; // in pixels, relative to the top-left corner
		Vector2f sorting_pivot; // in pixels, relative to the top-left corner
		uint8_t sorting_layer = (uint8_t)map::get_object_layer_index();
		Color color = colors::WHITE;
		unsigned int flags = TILE_VISIBLE;

		void set_texture_rect(unsigned int id);
		bool set_tileset(Handle<tiled::Tileset> handle);
		bool set_tileset(const std::string& tileset_name);
		bool set_tile(unsigned int id);
		void set_flag(unsigned int flag, bool value);
		bool get_flag(unsigned int flag) const;
		void set_rotation(int clockwise_quarter_turns);
	};

	struct TileAnimation
	{
		float progress = 0.f; // aka normalized time, in the range [0, 1]
		float speed = 1.f;
		unsigned int frame = 0; // index into tiled::Tile::animation
		bool loop = true;
	};

	void update_tile_positions(float dt);
	void update_tile_animations(float dt);
	void add_tile_sprites_for_drawing(const Vector2f& camera_min, const Vector2f& camera_max);

	Tile& emplace_tile(entt::entity entity);
	Tile* get_tile(entt::entity entity);
	bool remove_tile(entt::entity entity);

	TileAnimation& emplace_tile_animation(entt::entity entity);
}
