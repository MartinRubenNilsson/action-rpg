#include "ecs_physics.h"
#include "physics.h"
#include "ecs_common.h"
#include "audio.h"

namespace ecs
{
	struct ContactListener : b2ContactListener // forward declaration
	{
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;
	};

	extern entt::registry _registry;
	ContactListener _contact_listener;

	void _on_destroy_b2Body_ptr(entt::registry& registry, entt::entity entity) {
		physics::destroy_body(registry.get<b2Body*>(entity));
	}

	void initialize_physics()
	{
		physics::set_contact_listener(&_contact_listener);
		_registry.on_destroy<b2Body*>().connect<_on_destroy_b2Body_ptr>();
	}

	void ContactListener::BeginContact(b2Contact* contact)
	{
		b2Fixture* fixture_a = contact->GetFixtureA();
		b2Fixture* fixture_b = contact->GetFixtureB();
		b2Body* body_a = fixture_a->GetBody();
		b2Body* body_b = fixture_b->GetBody();
		entt::entity entity_a = (entt::entity)body_a->GetUserData().pointer;
		entt::entity entity_b = (entt::entity)body_b->GetUserData().pointer;
		std::string type_a = get_type(entity_a);
		std::string type_b = get_type(entity_b);

		// If type_a compares lexiographically greater than type_b, swap everything.
		// This reduces the number of cases we have to check for.
		if (type_a.compare(type_b) > 0)
		{
			std::swap(fixture_a, fixture_b);
			std::swap(body_a, body_b);
			std::swap(entity_a, entity_b);
			std::swap(type_a, type_b);
		}

		if (type_a == "enemy" && type_b == "player")
		{
			audio::play_sound("150_00");
			mark_for_destruction(entity_a);
			mark_for_destruction(entity_b);
		}
	}

	void ContactListener::EndContact(b2Contact* contact)
	{
		// TODO
	}
}
