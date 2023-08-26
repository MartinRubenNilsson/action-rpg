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
		debug_draw.SetTileSize(sf::Vector2u(16, 16)); // TODO: remove hardcoded value
		_world->SetDebugDraw(&debug_draw);
		_world->DebugDraw();
	}

	b2World& get_world() {
		return *_world;
	}

	b2Body* create_static_aabb(
		const sf::FloatRect& aabb,
		uintptr_t user_data,
		bool sensor)
	{
		b2BodyDef body_def;
		body_def.type = b2_staticBody;
		body_def.position.x = aabb.left;
		body_def.position.y = aabb.top;
		body_def.userData.pointer = user_data;

		b2Body* body = _world->CreateBody(&body_def);
		if (!body)
			return nullptr;

		b2PolygonShape shape;
		shape.SetAsBox(
			aabb.width / 2.f, aabb.height / 2.f,
			b2Vec2(aabb.width / 2.f, aabb.height / 2.f),
			0.f);

		b2FixtureDef fixture_def;
		fixture_def.shape = &shape;
		fixture_def.isSensor = sensor;

		body->CreateFixture(&fixture_def);

		return body;
	}
}