#pragma once

namespace physics
{
	void initialize();
	void shutdown();
	void update(float dt);
	void debug_draw(sf::RenderWindow& window);

	b2World& get_world();
	b2Body* create_static_aabb(const sf::FloatRect& aabb, uintptr_t user_data = 0);
}

