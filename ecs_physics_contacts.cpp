#include "stdafx.h"
#include "ecs_physics_contacts.h"
#include "physics_helpers.h"
#include "audio.h"
#include "ecs_common.h"
#include "ecs_player.h"

namespace ecs
{
	void on_begin_contact(const PhysicsContact& contact)
	{
		b2Fixture* fixture_a = contact.fixture_a;
		b2Fixture* fixture_b = contact.fixture_b;
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
				hurt_player(entity_a, 1);
			}
		}
	}
}

