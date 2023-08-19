#pragma once

namespace game
{
	void update(entt::registry& registry, float dt);
	void render(const entt::registry& registry, sf::RenderWindow& window);
}

