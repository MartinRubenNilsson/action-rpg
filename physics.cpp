#include "physics.h"
#include "physics_debug.h"

#define PHYSICS_GRAVITY b2Vec2(0, 0)
#define PHYSICS_TIME_STEP (1.f / 60.f)
#define PHYSICS_VELOCITY_ITERATIONS 8
#define PHYSICS_POSITION_ITERATIONS 3

namespace physics
{
	std::unique_ptr<b2World> _world;
	float _time_accumulator = 0.f;

	void initialize() {
		_world = std::make_unique<b2World>(PHYSICS_GRAVITY);
	}

	void shutdown() {
		_world.reset();
	}

	void update(float dt)
	{
		_time_accumulator += dt;
		while (_time_accumulator > PHYSICS_TIME_STEP)
		{
			_time_accumulator -= PHYSICS_TIME_STEP;
			_world->Step(
				PHYSICS_TIME_STEP,
				PHYSICS_VELOCITY_ITERATIONS,
				PHYSICS_POSITION_ITERATIONS);
		}
	}

	void debug_draw(sf::RenderTarget& render_target)
	{
		DebugDrawSFML debug_draw(render_target);
		debug_draw.SetFlags(b2Draw::e_shapeBit);
		_world->SetDebugDraw(&debug_draw);
		_world->DebugDraw();
	}

	void set_contact_filter(b2ContactFilter* filter) {
		_world->SetContactFilter(filter);
	}

	void set_contact_listener(b2ContactListener* listener) {
		_world->SetContactListener(listener);
	}

	b2Body* create_body(const b2BodyDef* body_def) {
		return _world->CreateBody(body_def);
	}

	void destroy_body(b2Body* body) {
		_world->DestroyBody(body);
	}
}