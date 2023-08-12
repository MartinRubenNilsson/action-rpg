#include "game.h"

namespace game
{
	entt::entity player_entity = entt::null;

	void update_player(entt::registry& registry, float dt)
	{
		if (!registry.valid(player_entity))
			return;

		// Set the player's movement direction based on which keys are pressed.
		sf::Vector2f direction;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			direction.x -= 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			direction.x += 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			direction.y -= 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			direction.y += 1;

		// Update the player's position based on their velocity.
		auto& sprite = registry.get<sf::Sprite>(player_entity);
		sprite.move(direction * dt * 100.0f);
	}
}
