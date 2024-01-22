#include "ecs_physics.h"
#include "physics_debug.h"
#include "physics_helpers.h"
#include "audio.h"
#include "map.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_player.h"

namespace ecs
{
	struct ContactListener : b2ContactListener // forward declaration
	{
		void BeginContact(b2Contact* contact) override;
	};

	const b2Vec2 PHYSICS_GRAVITY(0, 0);
	const float PHYSICS_TIME_STEP = 1.f / 60.f;
	const int PHYSICS_VELOCITY_ITERATIONS = 8;
	const int PHYSICS_POSITION_ITERATIONS = 3;

	extern entt::registry _registry;
	ContactListener _contact_listener;
	std::unique_ptr<b2World> _world;
	float _physics_time_accumulator = 0.f;

	void _on_destroy_b2Body_ptr(entt::registry& registry, entt::entity entity) {
		_world->DestroyBody(registry.get<b2Body*>(entity));
	}

	void initialize_physics()
	{
		_world = std::make_unique<b2World>(PHYSICS_GRAVITY);
		_world->SetContactListener(&_contact_listener);
		_registry.on_destroy<b2Body*>().connect<_on_destroy_b2Body_ptr>();
	}

	void shutdown_physics()
	{
		_registry.on_destroy<b2Body*>().disconnect<_on_destroy_b2Body_ptr>();
		_world.reset();
	}

	void update_physics(float dt)
	{
		_physics_time_accumulator += dt;
		while (_physics_time_accumulator > PHYSICS_TIME_STEP) {
			_physics_time_accumulator -= PHYSICS_TIME_STEP;
			_world->Step(
				PHYSICS_TIME_STEP,
				PHYSICS_VELOCITY_ITERATIONS,
				PHYSICS_POSITION_ITERATIONS);
		}
	}

	void render_physics(sf::RenderTarget& target)
	{
		physics::DebugDrawSFML debug_draw(target);
		debug_draw.SetFlags(b2Draw::e_shapeBit);
		_world->SetDebugDraw(&debug_draw);
		_world->DebugDraw();
		_world->SetDebugDraw(nullptr);
	}

	b2Body* emplace_body(entt::entity entity, const b2BodyDef& body_def)
	{
		b2BodyDef body_def_copy = body_def;
		body_def_copy.userData.pointer = (uintptr_t)entity;
		b2Body* body = _world->CreateBody(&body_def_copy);
		return _registry.emplace_or_replace<b2Body*>(entity, body);
	}

	void remove_body(entt::entity entity) {
		_registry.remove<b2Body*>(entity);
	}

	std::vector<entt::entity> query_aabb(const sf::Vector2f& min, const sf::Vector2f& max)
	{
		struct QueryCallback : public b2QueryCallback
		{
			std::vector<b2Fixture*> fixtures;

			bool ReportFixture(b2Fixture* fixture) override
			{
				fixtures.push_back(fixture);
				return true;
			}
		};

		QueryCallback callback;
		b2AABB aabb;
		aabb.lowerBound = b2Vec2(min.x, min.y);
		aabb.upperBound = b2Vec2(max.x, max.y);
		_world->QueryAABB(&callback, aabb);
		std::vector<entt::entity> entities;
		for (b2Fixture* fixture : callback.fixtures) {
			entt::entity entity = get_entity(fixture->GetBody());
			if (_registry.valid(entity))
				entities.push_back(entity);
		}
		std::sort(entities.begin(), entities.end());
		entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
		return entities;
	}

	void ContactListener::BeginContact(b2Contact* contact)
	{
		// WARNING: You cannot create/destroy Box2D entities inside this callback.

		b2Fixture* fixture_a = contact->GetFixtureA();
		b2Fixture* fixture_b = contact->GetFixtureB();
		b2Body* body_a = fixture_a->GetBody();
		b2Body* body_b = fixture_b->GetBody();
		entt::entity entity_a = get_entity(body_a);
		entt::entity entity_b = get_entity(body_b);
		std::string class_a = get_class(entity_a);
		std::string class_b = get_class(entity_b);
		if (class_a.empty() && class_b.empty()) return;

		// Sort the classes alphabetically; this reduces the number of cases we need to handle.
		if (class_a.compare(class_b) > 0) {
			std::swap(fixture_a, fixture_b);
			std::swap(body_a, body_b);
			std::swap(entity_a, entity_b);
			std::swap(class_a, class_b);
		}

		if (class_a.empty()) {
			// TODO
		} else if (class_a == "pickup") {
			if (class_b == "player") {
				audio::play("event:/snd_pickup");
				destroy_at_end_of_frame(entity_a);
				return;
			}
		} else if (class_a == "player") {
			if (class_b == "trigger") {
				std::string string;
				if (get_string(entity_b, "map", string)) {
					if (map::open(string, true)) {
						if (get_string(entity_b, "spawnpoint", string))
							map::set_player_spawnpoint(string);
					}
				}
			}
		} else if (class_a == "arrow") {
			if (class_b == "slime") {
				destroy_slime(entity_b);
				destroy_at_end_of_frame(entity_a);
			}
		}
	}

	void destroy_slime(entt::entity slime_entity) {
		// Check if the entity is valid before attempting to destroy it
		if (!_registry.valid(slime_entity)) {
			return;
		}

		// Optional: Perform any other game logic updates, such as increasing the score

		// Destroy the entity
		destroy_at_end_of_frame(slime_entity);
	}
}
