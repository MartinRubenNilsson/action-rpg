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
}