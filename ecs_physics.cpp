#include "ecs_physics.h"
#include "physics.h"
#include "ecs_common.h"
#include "ecs.h"

namespace ecs
{
	struct ContactListener : b2ContactListener // forward declaration
	{
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;
	};

	extern entt::registry _registry;
	ContactListener _contact_listener;
	std::vector<b2Contact*> _begin_contacts;
	std::vector<b2Contact*> _end_contacts;

	void ContactListener::BeginContact(b2Contact* contact) {
		_begin_contacts.push_back(contact);
	}

	void ContactListener::EndContact(b2Contact* contact) {
		_end_contacts.push_back(contact);
	}

	void _on_destroy_b2Body_ptr(entt::registry& registry, entt::entity entity)
	{
		auto body = registry.get<b2Body*>(entity);
		body->GetWorld()->DestroyBody(body);
	}

	void initialize_physics()
	{
		physics::get_world().SetContactListener(&_contact_listener);
		_registry.on_destroy<b2Body*>().connect<_on_destroy_b2Body_ptr>();
	}

	void update_physics()
	{
		for (b2Contact* contact : _begin_contacts)
		{
			b2Fixture* fixture_a = contact->GetFixtureA();
			b2Fixture* fixture_b = contact->GetFixtureB();
			b2Body* body_a = fixture_a->GetBody();
			b2Body* body_b = fixture_b->GetBody();
			entt::entity entity_a = (entt::entity)body_a->GetUserData().pointer;
			entt::entity entity_b = (entt::entity)body_b->GetUserData().pointer;

			//TODO: move this to callbacks

			if ((has_type(entity_a, "player") && has_type(entity_b, "trigger")) ||
				(has_type(entity_a, "trigger") && has_type(entity_b, "player")))
			{
				destroy_deferred(entity_a);
				destroy_deferred(entity_b);
			}
		}

		_begin_contacts.clear();
		_end_contacts.clear();
	}
}
