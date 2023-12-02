#include "ecs_graphics.h"
#include "defines.h"
#include "physics_helpers.h"
#include "tiled.h"

namespace ecs
{
	extern entt::registry _registry;

	void update_graphics(float dt)
	{
		for (auto [entity, anim] : _registry.view<Animation>().each()) {
			anim.time += anim.speed * dt;
			switch (anim.type)
			{
			case AnimationType::Default:
			{
				anim.tile_class = "";
				break;
			}
			case AnimationType::Player:
			{
				sf::Vector2f velocity;
				if (b2Body** body = _registry.try_get<b2Body*>(entity))
					velocity = get_linear_velocity(*body);
				if (is_zero(velocity))
				{
					anim.state = "idle";
					anim.time = 0;
				}
				else
				{
					anim.state = (length(velocity) > 7) ? "run" : "walk";
					anim.direction = get_direction(velocity);
				}
				anim.tile_class = anim.state + "_" + anim.direction;
				break;
			}
			}
		}

		for (auto [entity, anim, tile] : _registry.view<Animation, const tiled::Tile*>().each()) {
			if (!tile) continue;
			anim._current_tile = tile;
			if (const tiled::Tile* new_tile = tiled::find_tile_by_class(tile->tileset->tiles, anim.tile_class))
				anim._current_tile = new_tile;
			uint32_t time_in_ms = (uint32_t)(anim.time * 1000.f);
			if (const tiled::Tile* new_tile = tiled::sample_animation(anim._current_tile->animation, time_in_ms))
				anim._current_tile = new_tile;
		}

		for (auto [entity, sprite, anim] : _registry.view<Sprite, const Animation>().each()) {
			if (!anim._current_tile) continue;
			sprite.sprite.setTextureRect(anim._current_tile->sprite.getTextureRect());
		}

		for (auto [entity, sprite, body] : _registry.view<Sprite, b2Body*>().each()) {
			sprite.sprite.setPosition(get_position(body) * PIXELS_PER_METER);
		}
	}

	void emplace_sprite(entt::entity entity, const Sprite& sprite) {
		_registry.emplace_or_replace<Sprite>(entity, sprite);
	}

	void emplace_animation(entt::entity entity, const Animation& anim) {
		_registry.emplace_or_replace<Animation>(entity, anim);
	}
}
