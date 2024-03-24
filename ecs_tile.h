#pragma once
#include "timer.h"

class Properties;

namespace tiled
{
	struct Tile;
}

namespace ecs
{
	enum class SortingLayer : uint8_t
	{
		Background1,
		Background2,
		Objects,
		Foreground1,
		Foreground2,
		Collision,
		Count
	};

	SortingLayer layer_name_to_sorting_layer(const std::string& name);

	enum TileFlags
	{
		TF_NONE           = 0,
		TF_VISIBLE        = 1 << 0, // if not set, the tile is not rendered
		TF_FLIP_X         = 1 << 1, // flip the sprite horizontally
		TF_FLIP_Y         = 1 << 2, // flip the sprite vertically
		TF_FLIP_DIAGONAL  = 1 << 3, // flip the bottom-left and top-right corners
		TF_ROTATE_90      = TF_FLIP_X | TF_FLIP_DIAGONAL,
		TF_ROTATE_180     = TF_FLIP_X | TF_FLIP_Y,
		TF_ROTATE_270     = TF_FLIP_Y | TF_FLIP_DIAGONAL,
		TF_LOOP           = 1 << 4, // loop the animation
		TF_FLIP_X_ON_LOOP = 1 << 5, // flip the sprite horizontally when the animation loops
		TF_NEW_FRAME      = 1 << 6, // the animation frame changed last update
		TF_LOOPED	      = 1 << 7, // the animation looped last update
	};

	class Tile
	{
	public:
		Tile() = default;
		Tile(const tiled::Tile* tile);

		std::shared_ptr<sf::Texture> texture; // optional; if not set, the tileset texture is used
		std::shared_ptr<sf::Shader> shader; // optional; if not set, the default shader is used
		sf::Vector2f position; // in pixels
		sf::Vector2f pivot; // in pixels, relative to the top-left corner
		sf::Vector2f sorting_pivot; // in pixels, relative to the top-left corner
		SortingLayer sorting_layer = SortingLayer::Objects;
		sf::Color color = sf::Color::White;
		Timer animation_timer;
		float animation_speed = 1.f;

		bool is_valid() const { return _tile; }
		bool set_tile(const std::string& class_); // uses the current tileset
		bool set_tile(const std::string& class_, const std::string& tileset_name);
		std::shared_ptr<sf::Texture> get_texture() const;
		sf::IntRect get_texture_rect(bool account_for_animation = false) const;
		const std::string& get_class(bool account_for_animation = false) const;
		const std::string& get_tileset_name() const;
		const Properties& get_properties(bool account_for_animation = false) const;
		bool has_animation() const;
		void update_animation(float dt);
		float get_animation_duration() const; // in seconds
		void set_flag(TileFlags flag, bool value);
		bool get_flag(TileFlags flag) const;

	private:
		const tiled::Tile* _tile = nullptr;
		uint32_t _animation_duration_ms = 0;
		uint32_t _animation_frame_index = 0;
		uint32_t _flags = TF_VISIBLE | TF_LOOP;

		bool _set_tile(const tiled::Tile* tile);
		const tiled::Tile* _get_tile(bool account_for_animation) const;
	};

	void update_tiles(float dt);

	bool has_tile(entt::entity entity);
	Tile& emplace_tile(entt::entity entity); // emplaces an invalid tile
	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile);
	Tile& get_tile(entt::entity entity); // WARNING: undefined behavior if the entity does not have a tile component
	Tile* try_get_tile(entt::entity entity);
	bool remove_tile(entt::entity entity);
}
