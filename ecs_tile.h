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
		Tile(const tiled::Tile* tile);

		sf::Vector2f position;
		sf::Vector2f pivot; // in pixels
		sf::Color color = sf::Color::White;
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
		
		sf::Sprite get_sprite() const;

		bool set_class(const std::string& class_);
		bool set_class_and_tileset(const std::string& class_, const std::string& tileset_name);
		std::string get_class() const; 
		std::string get_tileset_name() const;

		bool is_animated() const;
		float get_animation_duration() const;
		void update_animation(float dt);
		//size_t get_animation_loop_count() const { return _animation_loop_count; }

	private:
		const tiled::Tile* _tile = nullptr; // current tile
		const tiled::Tile* _frame = nullptr; // current animation frame
		uint32_t _animation_duration_ms = 0;
		uint32_t _animation_loop_count = 0;

		void initialize_animation();
	};

	void update_tiles(float dt);

	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile);
	Tile* emplace_tile(entt::entity entity, const std::string& tileset_name, const std::string& tile_class);
	void remove_tile(entt::entity entity);
}
