#include "game.h"
#include "ecs_tiles.h"
#include "math_vectors.h"

namespace game
{
	entt::entity _player_entity = entt::null;

	void set_player_entity(entt::entity entity)
	{
		_player_entity = entity;
	}

	void _update_player(entt::registry& registry, float dt)
	{
		if (!registry.valid(_player_entity))
			return;

		auto& tile = registry.get<ecs::Tile>(_player_entity);
		auto& sprite = registry.get<sf::Sprite>(_player_entity);

		// Set the player's direction based on which keys are pressed.
		sf::Vector2f velocity;
		char direction_char = tile.get_type().empty() ? ' ' : tile.get_type().back();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			velocity.x -= 1;
			direction_char = 'l';
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			velocity.x += 1;
			direction_char = 'r';
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			velocity.y -= 1;
			direction_char = 'u';
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			velocity.y += 1;
			direction_char = 'd';
		}

		sf::normalize_safe(velocity);

		std::string type = "idle";
		if (sf::is_zero(velocity))
		{
			tile.animation_time = 0;
		}
		else
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
			{
				type = "run";
				velocity *= 160.f;
			}
			else
			{
				type = "walk";
				velocity *= 90.f;
			}
		}
		tile.set_type(type + "_" + direction_char);
		sprite.move(velocity * dt);
	}

	void _update_tiles(entt::registry& registry, float dt)
	{
		for (auto [entity, tile, sprite] : 
			registry.view<ecs::Tile, sf::Sprite>().each())
		{
			if (tile.has_animation())
				tile.animation_time += dt;
			sprite.setTextureRect(tile.get_texture_rect());
		}
	}

	void update(entt::registry& registry, float dt)
	{
		_update_player(registry, dt);
		_update_tiles(registry, dt);
	}

	void render(const entt::registry& registry, sf::RenderWindow& window)
	{
		// Draw all sprites.
		for (auto [entity, sprite] : registry.view<sf::Sprite>().each())
			window.draw(sprite);
	}
}
