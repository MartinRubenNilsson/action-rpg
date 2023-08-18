#include "game.h"
#include "ecs_tiles.h"

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
		sf::Vector2f direction;
		char direction_char = tile.get_type().empty() ? ' ' : tile.get_type().back();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			direction.x -= 1;
			direction_char = 'l';
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			direction.x += 1;
			direction_char = 'r';
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			direction.y -= 1;
			direction_char = 'u';
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			direction.y += 1;
			direction_char = 'd';
		}

		// Normalize the direction vector.
		if (direction.x != 0 || direction.y != 0)
			direction = direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);

		std::string action = "idle";
		if (direction.x != 0 || direction.y != 0)
			action = "walk";
		tile.set_type(action + "_" + direction_char);

		// Update the player's position based on their direction.
		sprite.move(direction * dt * 80.0f);

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
