#pragma once

namespace ecs
{
	void initialize();
	void shutdown();
	void update(float dt);
	void render(sf::RenderWindow& window);

	entt::registry& get_registry();

	// Mark an entity for destruction at the end of the frame.
	// Calling this function is almost always preferable
	// to calling entt::registry::destroy directly.
	void destroy_deferred(entt::entity entity);
}

