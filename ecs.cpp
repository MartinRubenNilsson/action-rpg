#include "stdafx.h"
#include "ecs.h"
#include "console.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_player.h"
#include "ecs_ai.h"
#include "ecs_tile.h"
#include "ecs_camera.h"
#include "ecs_vfx.h"
#include "ecs_pickups.h"
#include "ecs_bomb.h"

namespace ecs
{
	int debug_flags = DEBUG_NONE;

	entt::registry _registry;

	void initialize() {
		initialize_physics();
	}

	void shutdown()
	{
		clear();
		shutdown_physics();
	}

	void process_event(const sf::Event& event) {
		process_event_player(event);
	}

	void update(float dt)
	{
		update_players(dt);
		update_pickups(dt);
		update_bombs(dt);	
		update_ai_logic(dt);
		update_ai_graphics(dt);
		update_physics(dt);
		update_common(dt);
		update_tiles(dt);
		update_vfx(dt);
		update_cameras(dt);
	}
	 
	void render(sf::RenderTarget& target)
	{
		// Get and set the current view.
		sf::View view = ecs::get_blended_camera_view();
		target.setView(view);

		// Compute the view bounds (assuming no rotation).
		sf::FloatRect view_bounds(
			view.getCenter() - view.getSize() / 2.f, // top left
			view.getSize());

		struct SortedSprite
		{
			sf::Sprite sprite;
			std::shared_ptr<sf::Shader> shader;
			SortingLayer sorting_layer;
			sf::Vector2f sorting_pos;

			bool operator<(const SortedSprite& other) const
			{
				if (sorting_layer != other.sorting_layer) return sorting_layer < other.sorting_layer;
				if (sorting_pos.y != other.sorting_pos.y) return sorting_pos.y < other.sorting_pos.y;
				if (sorting_pos.x != other.sorting_pos.x) return sorting_pos.x < other.sorting_pos.x;
				return false;
			}
		};

		// Collect all visible sprites in view.
		std::vector<SortedSprite> sprites;
		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			if (!tile.visible) continue;
			sf::Sprite sprite = tile.get_sprite();
			sf::FloatRect sprite_bounds = sprite.getGlobalBounds();
			if (!view_bounds.intersects(sprite_bounds)) continue;
			sf::Vector2f sorting_pos = sprite_bounds.getPosition() + tile.sorting_pivot;
			sprites.emplace_back(sprite, tile.shader, tile.sorting_layer, sorting_pos);
		}

		// Sort sprites by layer and position.
		std::sort(sprites.begin(), sprites.end());

		// Draw sprites.
		for (const SortedSprite& sorted_sprite : sprites) {
			if (sorted_sprite.shader)
				target.draw(sorted_sprite.sprite, sorted_sprite.shader.get());
			else
				target.draw(sorted_sprite.sprite);
		}

		// DRAW VFX

		for (auto [entity, vfx] : _registry.view<Vfx>().each())
			target.draw(vfx.sprite); // TODO: culling

		// DEBUG DRAWING

		if (debug_flags & DEBUG_PIVOTS) {
			for (const SortedSprite& sorted_sprite : sprites) {
				if (sorted_sprite.sorting_layer != SortingLayer::Objects) continue;
				sf::CircleShape circle(1.f);
				circle.setPosition(sorted_sprite.sorting_pos);
				circle.setFillColor(sf::Color::Red);
				target.draw(circle);
			}
		}
		if (debug_flags & DEBUG_PHYSICS)
			debug_draw_physics();
		if (debug_flags & DEBUG_AI)
			debug_draw_ai();
		if (debug_flags & DEBUG_PLAYER)
			debug_draw_players();
	}
}
