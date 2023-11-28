#pragma once

namespace ecs
{
	void initialize();
	void shutdown();
	void process_event(const sf::Event& event);
	void update(float dt);
	void render(sf::RenderWindow& window);
	entt::registry& get_registry();

	// Potentially unsafe to call while iterating over a view.
	void destroy_immediately(entt::entity entity); 
	// Safe to call while iterating over a view.
	void destroy_at_end_of_frame(entt::entity entity);
}

