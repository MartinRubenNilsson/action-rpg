#pragma once
#include "timer.h"

namespace tiled
{
	struct Tile;
	struct FlippedTile;
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
		Tile(const tiled::FlippedTile& tile);

		sf::Vector2f position;
		sf::Vector2f origin;
		sf::Color color = sf::Color::White;
		bool flip_x = false;
		bool flip_y = false;
		bool visible = true;
		SortingLayer sorting_layer = SortingLayer::Objects;
		sf::Vector2f sorting_pivot = sf::Vector2f(0.5f, 1.f); //TODO: implement
		Timer animation_timer;
		float animation_speed = 1.f;
		bool animation_loop = true;
		
		sf::Sprite get_sprite() const;
		std::string get_class() const;
		bool set_class(const std::string& class_);
		bool is_animated() const;
		bool update_animation(float dt); // Returns true if the animation frame changed.

	private:
		const tiled::Tile* _tile = nullptr; // current tile
		const tiled::Tile* _frame = nullptr; // current animation frame

		uint32_t get_animation_duration() const; // in milliseconds
	};

	void update_graphics(float dt);

	Tile& emplace_tile(entt::entity entity, const tiled::FlippedTile& tile);
	Tile* emplace_tile(entt::entity entity, const std::string& tileset_name, const std::string& tile_class);
	void remove_tile(entt::entity entity);

	//TODO: remove
	struct SlimeAnimationController {};
	void emplace_slime_animation_controller(entt::entity entity);
}
