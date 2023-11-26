#pragma once

namespace ecs
{
	void initialize();
	void shutdown();
	void process_event(const sf::Event& event);
	void update(float dt);
	void render(sf::RenderWindow& window);
	entt::registry& get_registry();

	void destroy_at_end_of_frame(entt::entity entity);
}

