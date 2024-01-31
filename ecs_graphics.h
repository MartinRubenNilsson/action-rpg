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
		bool isPassable;

		SortingLayer sorting_layer = SortingLayer::Objects;
		sf::Vector2f sorting_pivot; // in pixels

		Timer animation_timer;
		float animation_speed = 1.f;
		bool animation_loop = true;
		
		sf::Sprite get_sprite() const;

		std::string get_class() const; 
		bool set_class(const std::string& class_);

		bool is_animated() const;
		void update_animation(float dt);
		bool animation_looped_this_frame() const { return _animation_looped_this_frame; }
		size_t get_animation_loop_count() const { return _animation_loop_count; }

	private:
		const tiled::Tile* _tile = nullptr; // current tile
		const tiled::Tile* _frame = nullptr; // current animation frame
		bool _animation_looped_this_frame = false;
		size_t _animation_loop_count = 0;

		uint32_t get_animation_duration_in_ms() const;
		void initialize_animation_state();
	};

	void update_graphics(float dt);

	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile);
	Tile* emplace_tile(entt::entity entity, const std::string& tileset_name, const std::string& tile_class);
	void remove_tile(entt::entity entity);

	//TODO: remove
	struct SlimeAnimationController {};
	void emplace_slime_animation_controller(entt::entity entity);
}
