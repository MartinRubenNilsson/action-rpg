#pragma once

namespace ecs
{
	enum DebugFlags
	{
		DEBUG_NONE    = 0,
		DEBUG_PIVOTS  = 1 << 0,
		DEBUG_PHYSICS = 1 << 1,
		DEBUG_AI      = 1 << 2,
	};

	extern int debug_flags;

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

