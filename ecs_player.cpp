#include "ecs_player.h"
#include "ecs_common.h"
#include "math_vectors.h"
#include "utility_b2.h"

extern bool window_has_focus;

namespace ecs
{
	extern entt::registry _registry;
	entt::entity _player_entity = entt::null;

	bool player_exists() {
		return _registry.valid(_player_entity);
	}

	entt::entity get_player_entity() {
		return _player_entity;
	}

	sf::Vector2f get_player_position() {
		if (auto body = _registry.try_get<b2Body*>(_player_entity))
			return b2::get_position(**body);
		return sf::Vector2f();
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

	void _update_player_velocity()
	{
		if (!_registry.valid(_player_entity))
			return;

		//TODO: if window don't have focus, don't update player

		sf::Vector2f velocity;
		if (window_has_focus)
		{
			velocity.x -= sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
			velocity.x += sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
			velocity.y -= sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
			velocity.y += sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
		}

		normalize_safe(velocity);

		if (window_has_focus && sf::Keyboard::isKeyPressed(sf::Keyboard::X))
			velocity *= 9.5f;
		else
			velocity *= 5.5f;

		b2Body& body = *_registry.get<b2Body*>(_player_entity);
		b2::set_linear_velocity(body, velocity);
	}

	void update_player()
	{
		_find_and_store_player_entity();
		_update_player_velocity();
	}
}

