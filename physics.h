#pragma once

class b2ContactListener;
class b2Body;

namespace physics
{
	void initialize();
	void shutdown();
	void update(float dt);
	void debug_draw(sf::RenderWindow& window);

	void set_contact_listener(b2ContactListener* listener);

	b2Body* create_body(const b2BodyDef* def);
	void destroy_body(b2Body* body);

	// The position of the body in the physics world will be top left corner of the AABB.
	b2Body* create_static_aabb(
		const sf::FloatRect& aabb,
		uintptr_t user_data = 0, // Should be set to the entity ID.
		bool sensor = false);
}

