#include "physics.h"
#include "physics_debug.h"

#define GRAVITY b2Vec2(0, 0)
#define TIME_STEP (1.f / 60.f)
#define VELOCITY_ITERATIONS 8
#define POSITION_ITERATIONS 3

namespace physics
{
	std::unique_ptr<b2World> _world;
	float _time_accumulator = 0.f;

	void initialize() {
		_world = std::make_unique<b2World>(GRAVITY);
	}

	void shutdown() {
		_world.reset();
	}

	void update(float dt)
	{
		_time_accumulator += dt;
		while (_time_accumulator > TIME_STEP)
		{
			_time_accumulator -= TIME_STEP;
			_world->Step(
				TIME_STEP,
				VELOCITY_ITERATIONS,
				POSITION_ITERATIONS);
		}
	}

	void debug_draw(sf::RenderWindow& window) {
		DebugDrawSFML debug_draw(window);
		debug_draw.SetFlags(b2Draw::e_shapeBit);
		debug_draw.SetTileSize(sf::Vector2u(16, 16));
		_world->SetDebugDraw(&debug_draw);
		_world->DebugDraw();
	}

	b2World& get_world() {
		return *_world;
	}

	b2Body* create_static_aabb(const sf::FloatRect& aabb, uintptr_t user_data)
	{
		b2BodyDef body_def;
		body_def.type = b2_staticBody;
		body_def.position.x = aabb.left + aabb.width / 2;
		body_def.position.y = aabb.top + aabb.height / 2;
		body_def.userData.pointer = user_data;

		b2Body* body = _world->CreateBody(&body_def);
		if (!body)
			return nullptr;

		b2PolygonShape shape;
		shape.SetAsBox(aabb.width / 2, aabb.height / 2);
		body->CreateFixture(&shape, 0.f);

		return body;
	}
}