#pragma once

class b2ContactFilter;
class b2ContactListener;
class b2Body;

namespace physics
{
	void initialize();
	void shutdown();
	void update(float dt);
	void debug_draw(sf::RenderTarget& render_target);

	void set_contact_filter(b2ContactFilter* filter);
	void set_contact_listener(b2ContactListener* listener);

	b2Body* create_body(const b2BodyDef* body_def);
	void destroy_body(b2Body* body);
}

