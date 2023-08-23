#pragma once

namespace ecs
{
	void initialize();
	void shutdown();
	void update(float dt);
	void render(sf::RenderWindow& window);

	entt::registry& get_registry();
}

