#include "ecs_physics.h"
#include "physics.h"
#include "audio.h"
#include "map.h"
#include "ecs_common.h"

namespace ecs
{
	struct ContactFilter : b2ContactFilter // forward declaration
	{
		bool ShouldCollide(b2Fixture* fixture_a, b2Fixture* fixture_b) override;
	};

	struct ContactListener : b2ContactListener // forward declaration
	{
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;
	};

	extern entt::registry _registry;
	ContactFilter _contact_filter;
	ContactListener _contact_listener;

	void _on_destroy_b2Body_ptr(entt::registry& registry, entt::entity entity) {
		physics::destroy_body(registry.get<b2Body*>(entity));
	}

	void initialize_physics()
	{
		physics::set_contact_filter(&_contact_filter);
		physics::set_contact_listener(&_contact_listener);
		_registry.on_destroy<b2Body*>().connect<_on_destroy_b2Body_ptr>();
	}

	std::vector<entt::entity> query_aabb(const sf::Vector2f& min, const sf::Vector2f& max)
	{
		std::vector<entt::entity> entities;
		for (b2Fixture* fixture : physics::query_aabb(min, max))
			entities.push_back((entt::entity)fixture->GetBody()->GetUserData().pointer);
		std::sort(entities.begin(), entities.end());
		entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
		return entities;
	}

	bool ContactFilter::ShouldCollide(b2Fixture* fixture_a, b2Fixture* fixture_b) {
		return true;
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
		if (type_a.empty() || type_b.empty())
			return;

		// Sort the types alphabetically; this reduces
		// the number of cases we need to handle.
		if (type_a.compare(type_b) > 0)
		{
			std::swap(fixture_a, fixture_b);
			std::swap(body_a, body_b);
			std::swap(entity_a, entity_b);
			std::swap(type_a, type_b);
		}

		if (type_a == "enemy" && type_b == "player")
		{
			//audio::play_sound("150_00");
			mark_for_destruction(entity_a);
			mark_for_destruction(entity_b);
		}

		if (type_a == "player" && type_b == "trigger_map")
		{
			std::string map_name;
			if (get_string(entity_b, "map", map_name))
				map::open(map_name);
		}
	}

	void ContactListener::EndContact(b2Contact* contact) {
		// Not implemented.
	}
}
