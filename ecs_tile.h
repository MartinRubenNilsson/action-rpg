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

	class Tile
	{
	public:
		Tile() = default;
		Tile(const tiled::Tile* tile);

		sf::Vector2f position;
		sf::Vector2f pivot; // in pixels
		sf::Color color = sf::Color::White;
		std::shared_ptr<sf::Texture> texture; // optional
		std::shared_ptr<sf::Shader> shader; // optional
		bool visible = true;
		bool flip_x = false;
		bool flip_y = false;

		SortingLayer sorting_layer = SortingLayer::Objects;
		sf::Vector2f sorting_pivot; // in pixels

		Timer animation_timer;
		float animation_speed = 1.f;
		bool animation_loop = true;
		bool animation_flip_x_on_loop = false;

		bool is_valid() const { return _tile; }
		bool set_sprite(const std::string& tile_class); // uses the current tileset
		bool set_sprite(const std::string& tile_class, const std::string& tileset_name);
		sf::Sprite get_sprite() const;
		std::string get_tile_class() const; 
		std::string get_tileset_name() const;

		bool is_animated() const;
		void update_animation(float dt);
		float get_animation_duration() const; // in seconds

	private:
		const tiled::Tile* _tile = nullptr;
		uint32_t _animation_duration_ms = 0;
		uint32_t _animation_frame_index = 0;
		uint32_t _animation_loop_count = 0; // unused right now

		bool _set_sprite(const tiled::Tile* tile);
	};

	void update_tiles(float dt);

	Tile& emplace_tile(entt::entity entity); // emplaces an invalid tile
	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile);
	Tile& get_tile(entt::entity entity);
	Tile* try_get_tile(entt::entity entity);
	bool remove_tile(entt::entity entity);
}
