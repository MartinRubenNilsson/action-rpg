#pragma once

namespace ecs
{
	void initialize();
	void shutdown();
	void process_event(const sf::Event& event);
	void update(float dt);
	void render(sf::RenderWindow& window);
	entt::registry& get_registry();

	// Marks an entity to be destroyed at the end of the frame.
	void mark_for_destruction(entt::entity entity);
}

