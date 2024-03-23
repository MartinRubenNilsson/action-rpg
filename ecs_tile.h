#pragma once
#include "timer.h"

namespace tiled
{
	struct Tile;
}

namespace ecs
{
	enum class SortingLayer
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
		TF_VISIBLE        = 1 << 0,
		TF_FLIP_X         = 1 << 1,
		TF_FLIP_Y         = 1 << 2,
		TF_LOOP           = 1 << 3, // loop the animation
		TF_FLIP_X_ON_LOOP = 1 << 4, // flip the sprite horizontally when the animation loops
		TF_JUST_LOOPED	  = 1 << 5, // is set when the animation looped last update
	};

	class Tile
	{
	public:
		Tile() = default;
		Tile(const tiled::Tile* tile);

		sf::Vector2f position; // in pixels
		sf::Vector2f pivot; // in pixels
		sf::Vector2f sorting_pivot; // in pixels
		SortingLayer sorting_layer = SortingLayer::Objects;
		sf::Color color = sf::Color::White;
		std::shared_ptr<sf::Texture> texture; // optional
		std::shared_ptr<sf::Shader> shader; // optional
		Timer animation_timer;
		float animation_speed = 1.f;

		bool is_valid() const { return _tile; }
		bool set_sprite(const std::string& tile_class); // uses the current tileset
		bool set_sprite(const std::string& tile_class, const std::string& tileset_name);
		sf::Sprite get_sprite() const;
		std::string get_tile_class() const; 
		std::string get_tileset_name() const;
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

		bool _set_sprite(const tiled::Tile* tile);
	};

	void update_tiles(float dt);

	bool has_tile(entt::entity entity);
	Tile& emplace_tile(entt::entity entity); // emplaces an invalid tile
	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile);
	Tile& get_tile(entt::entity entity); // WARNING: undefined behavior if the entity does not have a tile component
	Tile* try_get_tile(entt::entity entity);
	bool remove_tile(entt::entity entity);
}
