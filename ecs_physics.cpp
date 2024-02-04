#include "ecs_physics.h"
#include "physics_debug.h"
#include "physics_helpers.h"
#include "audio.h"
#include "map.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_player.h"
#include "ecs_physics_filters.h"

namespace ecs
{
	const b2Vec2 _PHYSICS_GRAVITY(0, 0);
	const float _PHYSICS_TIME_STEP = 1.f / 60.f;
	const int _PHYSICS_VELOCITY_ITERATIONS = 8;
	const int _PHYSICS_POSITION_ITERATIONS = 3;

	struct ContactListener : b2ContactListener
	{
		struct Contact
		{
			b2Fixture* fixture_a = nullptr;
			b2Fixture* fixture_b = nullptr;
		};

		std::vector<Contact> contacts;

		void BeginContact(b2Contact* b2contact) override {
			Contact contact{};
			contact.fixture_a = b2contact->GetFixtureA();
			contact.fixture_b = b2contact->GetFixtureB();
			contacts.push_back(contact);
		}
	};

	extern entt::registry _registry;
	ContactListener _contact_listener;
	std::unique_ptr<b2World> _world;
	float _physics_time_accumulator = 0.f;

	void _on_destroy_b2Body_ptr(entt::registry& registry, entt::entity entity) {
		_world->DestroyBody(registry.get<b2Body*>(entity));
	}

	void initialize_physics()
	{
		_world = std::make_unique<b2World>(_PHYSICS_GRAVITY);
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
		// STEP PHYSICS WORLD

		_physics_time_accumulator += dt;
		while (_physics_time_accumulator > _PHYSICS_TIME_STEP) {
			_physics_time_accumulator -= _PHYSICS_TIME_STEP;
			_world->Step(
				_PHYSICS_TIME_STEP,
				_PHYSICS_VELOCITY_ITERATIONS,
				_PHYSICS_POSITION_ITERATIONS);
		}

		// HANDLE CONTACTS

		for (const auto& contact : _contact_listener.contacts) {
			b2Fixture* fixture_a = contact.fixture_a;
			b2Fixture* fixture_b = contact.fixture_b;
			b2Body* body_a = fixture_a->GetBody();
			b2Body* body_b = fixture_b->GetBody();
			entt::entity entity_a = get_entity(body_a);
			entt::entity entity_b = get_entity(body_b);
			std::string class_a = get_class(entity_a);
			std::string class_b = get_class(entity_b);
			if (class_a.empty() && class_b.empty()) continue;

			// Sort the classes alphabetically; this reduces the number of cases we need to handle.
			if (class_a.compare(class_b) > 0) {
				std::swap(fixture_a, fixture_b);
				std::swap(body_a, body_b);
				std::swap(entity_a, entity_b);
				std::swap(class_a, class_b);
			}

			if (class_a.empty()) {
				if (class_b == "arrow") {
					destroy_at_end_of_frame(entity_b);
				}
			} else if (class_a == "arrow") {
				if (class_b == "slime") {
					destroy_at_end_of_frame(entity_a);
					destroy_at_end_of_frame(entity_b);
				}
			} else if (class_a == "pickup") {
				if (class_b == "player") {
					audio::play("event:/snd_pickup");
					destroy_at_end_of_frame(entity_a);
				}
			} else if (class_a == "player") {
				if (class_b == "slime") {
					// Call function to hurt player
					int damage = 1; // decide how much damage a slime does
					hurt_player(entity_a, damage);
				} /*else if (class_b == "trigger") {
					std::string string;
					if (get_string(entity_b, "map", string)) {
						if (map::open(string, true)) {
							if (get_string(entity_b, "spawnpoint", string))
								map::set_player_spawnpoint(string);
						}
					}
				}*/
			}
		}

		_contact_listener.contacts.clear();
	}

	void debug_physics(sf::RenderTarget& target)
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
		body_def_copy.userData.entity = entity;
		b2Body* body = _world->CreateBody(&body_def_copy);
		_registry.emplace_or_replace<b2Body*>(entity, body);
		return body;
	}

	void remove_body(entt::entity entity) {
		_registry.remove<b2Body*>(entity);
	}

	std::vector<RayHit> raycast(const sf::Vector2f& ray_start, const sf::Vector2f& ray_end, uint16 mask_bits)
	{
		struct RayCastCallback : public b2RayCastCallback
		{
			std::vector<RayHit> hits;
			uint16 mask_bits = 0xFFFF;

			float ReportFixture(
				b2Fixture* fixture,
				const b2Vec2& point,
				const b2Vec2& normal,
				float fraction) override
			{
				uint16 category_bits = fixture->GetFilterData().categoryBits;
				if (!(category_bits & mask_bits)) return -1.f;
				RayHit hit{};
				hit.fixture = fixture;
				hit.body = fixture->GetBody();
				hit.entity = get_entity(hit.body);
				hit.point = sf::Vector2f(point.x, point.y);
				hit.normal = sf::Vector2f(normal.x, normal.y);
				hit.fraction = fraction;
				hits.push_back(hit);
				return 1.f;
			}
		};

		RayCastCallback callback;
		callback.mask_bits = mask_bits;
		_world->RayCast(&callback, b2Vec2(ray_start.x, ray_start.y), b2Vec2(ray_end.x, ray_end.y));
		return callback.hits;
	}

	std::vector<BoxHit> boxcast(const sf::Vector2f& box_min, const sf::Vector2f& box_max, uint16 mask_bits)
	{
		struct QueryCallback : public b2QueryCallback
		{
			std::vector<BoxHit> hits;
			uint16 mask_bits = 0xFFFF;

			bool ReportFixture(b2Fixture* fixture) override
			{
				uint16 category_bits = fixture->GetFilterData().categoryBits;
				if (!(category_bits & mask_bits)) return true;
				BoxHit hit{};
				hit.fixture = fixture;
				hit.body = fixture->GetBody();
				hit.entity = get_entity(hit.body);
				hits.push_back(hit);
				return true;
			}
		};

		QueryCallback callback;
		callback.mask_bits = mask_bits;
		_world->QueryAABB(&callback, b2AABB{ b2Vec2(box_min.x, box_min.y), b2Vec2(box_max.x, box_max.y) });
		return callback.hits;
	}
}
