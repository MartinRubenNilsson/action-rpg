#pragma once

namespace physics
{
	void initialize();
	void shutdown();
	void update(float dt);
	void debug_draw(sf::RenderWindow& window);

	b2World& get_world();

	// The position of the body in the physics world will be top left corner of the AABB.
	b2Body* create_static_aabb(
		const sf::FloatRect& aabb,
		uintptr_t user_data = 0, // Should be set to the entity ID.
		bool sensor = false);
}

