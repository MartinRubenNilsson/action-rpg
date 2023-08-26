#pragma once

namespace ecs
{
	void initialize();
	void shutdown();
	void update(float dt);
	void render(sf::RenderWindow& window);

	entt::registry& get_registry();

	// Mark an entity for destruction at the end of the frame.
	void destroy_deferred(entt::entity entity);
}

