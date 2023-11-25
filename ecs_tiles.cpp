#include "ecs_tiles.h"
#include "defines.h"
#include "physics_helpers.h"

namespace ecs
{
	extern entt::registry _registry;

	void _update_tile_types()
	{
		for (auto [entity, tile, body] :
			_registry.view<Tile, b2Body*>().each())
		{
			if (tile.tile->tileset->name == "naked_human")
			{
				sf::Vector2f velocity = get_linear_velocity(body);
				if (is_zero(velocity))
				{
					tile.action = "idle";
					tile.animation_time = 0; // should probably be done elsewhere
				}
				else
				{
					tile.action = (length(velocity) > 7) ? "run" : "walk";
					tile.direction = get_direction(velocity);
				}
			}
		}
	}

	void _update_tile_animations(float dt)
	{
		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			tile.animation_time += tile.animation_speed * dt;
			std::string current_class = tile.action + "_" + tile.direction;
			if (const tiled::Tile* current_tile = tile.tile->tileset->find_tile(current_class))
				if (const tiled::Tile* frame = current_tile->query_animation((uint32_t)(tile.animation_time * 1000.f)))
					tile.sprite = frame->sprite;
				else if (const tiled::Tile* frame = current_tile->query_animation((uint32_t)(tile.animation_time * 1000.f)))
					tile.sprite = frame->sprite;
		}
	}

	void _update_tile_sprite_positions()
	{
		for (auto [entity, tile, body] : _registry.view<Tile, b2Body*>().each()) {
			tile.sprite.setPosition(get_position(body) * PIXELS_PER_METER);
		}
	}

	void update_tiles(float dt)
	{
		_update_tile_types();
		_update_tile_animations(dt);
		_update_tile_sprite_positions();
	}
}
