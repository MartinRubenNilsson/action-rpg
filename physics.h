#pragma once

namespace physics
{
	void initialize();
	void shutdown();
	void update(float dt);
	void debug_draw(sf::RenderWindow& window);

	b2World& get_world();
}

