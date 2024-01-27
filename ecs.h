#pragma once

namespace ecs
{
	extern bool debug_draw_physics;
	extern bool debug_draw_pivots;

	void initialize();
	void shutdown();
	void clear();
	void process_event(const sf::Event& event);
	void update(float dt);
	void render(sf::RenderWindow& window);

	entt::entity create();
	entt::entity create(entt::entity hint);
	void destroy_immediately(entt::entity entity); 
	void destroy_at_end_of_frame(entt::entity entity);
}

