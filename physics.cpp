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

	void debug_draw(sf::RenderWindow& window) {
		DebugDrawSFML debug_draw(window);
		debug_draw.SetFlags(b2Draw::e_shapeBit);
		debug_draw.SetTileSize(sf::Vector2u(16, 16)); // TODO: remove hardcoded value
		_world->SetDebugDraw(&debug_draw);
		_world->DebugDraw();
	}

	void set_contact_listener(b2ContactListener* listener) {
		_world->SetContactListener(listener);
	}

	b2Body* create_body(const b2BodyDef* def) {
		return _world->CreateBody(def);
	}

	void destroy_body(b2Body* body) {
		_world->DestroyBody(body);
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