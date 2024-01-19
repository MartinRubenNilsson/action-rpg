#pragma once
#include "timer.h"

namespace tiled
{
	struct Tile;
}

namespace ecs
{
	enum class TileLayer
	{
		// TODO
	};

	class Tile
	{
	public:
		Tile(const tiled::Tile* tile);

		sf::Vector2f position;
		sf::Color color = sf::Color::White;
		float sort_order = 0.f; // Tiles with smaller sort order are drawn first.
		bool visible = true;
		Timer animation_timer;
		float animation_speed = 1.f;
		bool animation_loop = true;
		
		sf::Sprite get_sprite() const;
		std::string get_class() const;
		bool change_class(const std::string& class_);
		bool is_animated() const;
		bool update_animation(float dt); // Returns true if the animation frame changed.

	private:
		const tiled::Tile* _tile = nullptr; // current tile
		const tiled::Tile* _frame = nullptr; // current animation frame

		uint32_t get_animation_duration() const;
	};

	void update_graphics(float dt);

	Tile& emplace_tile(entt::entity entity, const tiled::Tile* tile);
	bool emplace_tile(entt::entity entity, const std::string& tileset_name, const std::string& tile_class);

	//TODO: remove
	struct SlimeAnimationController {};
	void emplace_slime_animation_controller(entt::entity entity);
}
