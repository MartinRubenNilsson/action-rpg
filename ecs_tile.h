#pragma once
#include "timer.h"
#include "properties.h"

namespace tiled
{
	struct Tile;
}

namespace ecs
{
	enum TILE_FLAGS : unsigned int
	{
		TILE_VISIBLE        = (1 << 0), // if not set, the tile is not rendered
		TILE_FLIP_X         = (1 << 1), // flip the sprite horizontally
		TILE_FLIP_Y         = (1 << 2), // flip the sprite vertically
		TILE_FLIP_DIAGONAL  = (1 << 3), // flip the bottom-left and top-right corners
		TILE_LOOP           = (1 << 4), // loop the animation
		TILE_FLIP_X_ON_LOOP = (1 << 5), // flip the sprite horizontally when the animation loops
		TILE_FRAME_CHANGED  = (1 << 6), // the animation frame changed last update
		TILE_LOOPED	        = (1 << 7), // the animation looped last update
	};

	class Tile
	{
	public:
		Tile();
		Tile(const tiled::Tile* tile);

		Handle<graphics::Texture> texture;
		Handle<graphics::Shader> shader;
		Vector2f position; // in pixels
		Vector2f pivot; // in pixels, relative to the top-left corner
		Vector2f sorting_pivot; // in pixels, relative to the top-left corner
		uint8_t sorting_layer = 0;
		Color color = colors::WHITE;
		Timer animation_timer;
		float animation_speed = 1.f;

		bool is_valid() const { return _tile; }
		bool set_tile(unsigned int id); // uses the current tileset
		bool set_tile(unsigned int id, const std::string& tileset_name);
		bool set_tile(unsigned int x, unsigned int y); // uses the current tileset
		bool set_tile(const std::string& class_); // uses the current tileset
		bool set_tile(const std::string& class_, const std::string& tileset_name);
		void get_texture_rect(unsigned int& left, unsigned int& top, unsigned int& width, unsigned int& height, bool account_for_animation = true) const;
		unsigned int get_id(bool account_for_animation = true) const;
		Vector2u get_coords(bool account_for_animation = true) const;
		const std::string& get_class(bool account_for_animation = true) const;
		const std::string& get_tileset_name() const;
		const Properties& get_properties(bool account_for_animation = true) const;
		bool has_animation() const;
		void update_animation(float dt);
		float get_animation_duration() const; // in seconds
		unsigned int get_animation_frame() const;
		void set_flag(unsigned int flag, bool value);
		bool get_flag(unsigned int flag) const;
		void set_rotation(int clockwise_quarter_turns);

	private:
		const tiled::Tile* _tile = nullptr; // TODO: remove this pointer and replace with Handle<Tileset> and tile_id
		unsigned int _animation_duration_ms = 0;
		unsigned int _animation_frame = 0;
		unsigned int _flags = TILE_VISIBLE | TILE_LOOP;

		bool _set_tile(const tiled::Tile* tile);
		const tiled::Tile* _get_tile(bool account_for_animation) const;
	};

	void update_tiles(float dt);
	void add_tile_sprites_to_render_queue(const Vector2f& camera_min, const Vector2f& camera_max);

	Tile& emplace_tile(entt::entity entity);
	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile);
	Tile* get_tile(entt::entity entity);
	bool remove_tile(entt::entity entity);
	bool has_tile(entt::entity entity);
}
