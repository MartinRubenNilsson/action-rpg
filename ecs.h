#pragma once

namespace ecs
{
	enum DebugFlags
	{
		DEBUG_NONE    = 0,
		DEBUG_TILES   = 1 << 0,
		DEBUG_PHYSICS = 1 << 1,
		DEBUG_AI      = 1 << 2,
		DEBUG_PLAYER  = 1 << 3,
	};

	extern int debug_flags;

	void initialize();
	void shutdown();
	void process_event(const sf::Event& event);
	void update(float dt);
	void draw(sf::RenderTarget& target);
	void debug_draw();
}

