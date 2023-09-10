#pragma once

namespace ecs
{
	void initialize();
	void shutdown();
	void update(float dt);
	void render(sf::RenderWindow& window);

	void clear_registry();
	entt::registry& get_registry();
}

