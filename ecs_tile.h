#pragma once
#include "timer.h"
#include "properties.h"

namespace tiled
{
	struct Tile;
}

namespace ecs
{
	enum TileFlags : uint32_t
	{
		TF_VISIBLE        = (1 << 0), // if not set, the tile is not rendered
		TF_FLIP_X         = (1 << 1), // flip the sprite horizontally
		TF_FLIP_Y         = (1 << 2), // flip the sprite vertically
		TF_FLIP_DIAGONAL  = (1 << 3), // flip the bottom-left and top-right corners
		TF_LOOP           = (1 << 4), // loop the animation
		TF_FLIP_X_ON_LOOP = (1 << 5), // flip the sprite horizontally when the animation loops
		TF_FRAME_CHANGED  = (1 << 6), // the animation frame changed last update
		TF_LOOPED	      = (1 << 7), // the animation looped last update
	};

	class Tile
	{
	public:
		Tile();
		Tile(const tiled::Tile* tile);

		std::shared_ptr<sf::Texture> texture; // optional; if not set, the tileset texture is used
		int shader_id = 0; // optional; if set, the shader with this ID is used
		sf::Vector2f position; // in pixels
		sf::Vector2f pivot; // in pixels, relative to the top-left corner
		sf::Vector2f sorting_pivot; // in pixels, relative to the top-left corner
		uint8_t sorting_layer = 0;
		sf::Color color = sf::Color::White;
		Timer animation_timer;
		float animation_speed = 1.f;

		bool is_valid() const { return _tile; }
		bool set_tile(uint32_t id); // uses the current tileset
		bool set_tile(uint32_t id, const std::string& tileset_name);
		bool set_tile(const std::string& class_); // uses the current tileset
		bool set_tile(const std::string& class_, const std::string& tileset_name);
		const std::shared_ptr<sf::Texture>& get_texture() const;
		sf::IntRect get_texture_rect(bool account_for_animation = true) const;
		const std::string& get_class(bool account_for_animation = true) const;
		const std::string& get_tileset_name() const;
		const Properties& get_properties(bool account_for_animation = true) const;
		bool has_animation() const;
		void update_animation(float dt);
		float get_animation_duration() const; // in seconds
		uint32_t get_animation_frame() const;
		void set_flag(TileFlags flag, bool value);
		bool get_flag(TileFlags flag) const;
		void set_rotation(int clockwise_quarter_turns);

	private:
		const tiled::Tile* _tile = nullptr;
		uint32_t _animation_duration_ms = 0;
		uint32_t _animation_frame = 0;
		uint32_t _flags = TF_VISIBLE | TF_LOOP;

		bool _set_tile(const tiled::Tile* tile);
		const tiled::Tile* _get_tile(bool account_for_animation) const;
	};

	void update_tiles(float dt);
	void draw_tiles(const sf::Vector2f& camera_min, const sf::Vector2f& camera_max);

	Tile& emplace_tile(entt::entity entity);
	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile);
	Tile& get_tile(entt::entity entity); // Undefined behavior if entity does not have a Tile component!
	Tile* try_get_tile(entt::entity entity);
	bool remove_tile(entt::entity entity);
	bool has_tile(entt::entity entity);
}
