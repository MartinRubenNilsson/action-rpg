#pragma once

namespace game
{
	void set_player_entity(entt::entity entity = entt::null);
	void update(entt::registry& registry, float dt);
	void render(const entt::registry& registry, sf::RenderWindow& window);
}

