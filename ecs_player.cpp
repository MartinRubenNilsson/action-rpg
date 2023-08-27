#include "ecs_player.h"
#include "ecs_common.h"
#include "ecs_tiles.h"
#include "math_vectors.h"

namespace ecs
{
	extern entt::registry _registry;
	entt::entity _player_entity = entt::null;

	entt::entity get_player_entity() {
		return _player_entity;
	}

	void _find_and_store_player_entity()
	{
		_player_entity = entt::null;
		for (auto [entity, object] :
			_registry.view<const tmx::Object*>().each())
		{
			if (object->getType() == "player")
			{
				_player_entity = entity;
				break;
			}
		}
	}

	void _update_player_velocity_and_tile()
	{
		if (!_registry.valid(_player_entity))
			return;

		auto& tile = _registry.get<ecs::Tile>(_player_entity);

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
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
			{
				type = "run";
				velocity *= 9.5f;
			}
			else
			{
				type = "walk";
				velocity *= 5.5f;
			}
		}
		tile.set_type(type + "_" + direction_char);

		// Apply the velocity to the player's physics body.
		auto& body = _registry.get<b2Body*>(_player_entity);
		body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
	}

	void update_player()
	{
		_find_and_store_player_entity();
		_update_player_velocity_and_tile();
	}
}

