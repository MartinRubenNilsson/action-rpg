#include "stdafx.h"
#include "ecs_physics_contacts.h"

#include "ecs_common.h"
#include "ecs_player.h"
#include "ecs_pickups.h"
#include "ecs_damage.h"
#include "ecs_portal.h"
#include "ecs_blade_trap.h"

#include "audio.h"
#include "console.h"

namespace ecs
{
	void on_begin_contact(const PhysicsContact& contact)
	{
		b2Fixture* fixture_a = contact.fixture_a;
		b2Fixture* fixture_b = contact.fixture_b;
		b2Body* body_a = fixture_a->GetBody();
		b2Body* body_b = fixture_b->GetBody();
		entt::entity entity_a = body_a->GetUserData().entity;
		entt::entity entity_b = body_b->GetUserData().entity;
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
				// TODO add audio::play("event:/snd_arrow_hit"); when we have nice animation for arrow hitting a wall
				destroy_at_end_of_frame(entity_b);
			}
		} else if (class_a == "arrow") {
			destroy_at_end_of_frame(entity_a);
			if (class_b == "slime" || class_b == "bomb") {
				apply_damage(entity_b, { .type = DamageType::Projectile, .amount = 1 });
			}
		} else if (class_a == "blade_trap") {
			if (class_b == "player") {
				apply_damage(entity_b, { .amount = 1 });
			}
		} else if (class_a == "pickup") {
			if (class_b == "player") {

				//TODO: put somewehere else in a helper function
				Player* player = get_player(entity_b);
				assert(player);
				Pickup* pickup = get_pickup(entity_a);
				assert(pickup);

				switch (pickup->type) {
				case PickupType::Arrow: {
					player->arrows++;
					audio::create_event({ .path = "event:/snd_pickup" });
				} break;
				case PickupType::Rupee: {
					player->rupees++;
					audio::create_event({ .path = "event:/snd_pickup_rupee" });
				} break;
				case PickupType::Bomb: {
					player->bombs++;
					audio::create_event({ .path = "event:/snd_pickup" });
				} break;
				case PickupType::Heart: {
					player->health = std::min(player->health + 1, player->max_health);
					//TODO
					//audio::play("event:/snd_pickup_heart");
				} break;
				}

				destroy_at_end_of_frame(entity_a);
			}
		} else if (class_a == "player") {
 			if (class_b == "portal") {
				activate_portal(entity_b);
			} else if (class_b == "slime") {
				apply_damage_to_player(entity_a, { DamageType::Melee, 1 });
			}
		}
	}

	void on_end_contact(const PhysicsContact& contact)
	{
		b2Fixture* fixture_a = contact.fixture_a;
		b2Fixture* fixture_b = contact.fixture_b;
		b2Body* body_a = fixture_a->GetBody();
		b2Body* body_b = fixture_b->GetBody();
		entt::entity entity_a = body_a->GetUserData().entity;
		entt::entity entity_b = body_b->GetUserData().entity;
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

		// TODO
	}
}

