#include "stdafx.h"
#include "ecs_physics_events.h"
#include "ecs_common.h"
#include "ecs_sprites.h"
#include "ecs_player.h"
#include "ecs_pickups.h"
#include "ecs_damage.h"
#include "ecs_portal.h"
#include "ecs_blade_trap.h"
#include "audio.h"
#include "console.h"

#define PAIR(a, b) ((uint64_t)a << 32 | (uint64_t)b)

namespace ecs
{

	void process_contact_begin_touch_event(b2ShapeId shape_a, b2ShapeId shape_b)
	{
		const b2BodyId body_a = b2Shape_GetBody(shape_a);
		const b2BodyId body_b = b2Shape_GetBody(shape_b);
		const entt::entity entity_a = (entt::entity)(uintptr_t)b2Body_GetUserData(body_a);
		const entt::entity entity_b = (entt::entity)(uintptr_t)b2Body_GetUserData(body_b);
		const Tag tag_a = get_tag(entity_a);
		const Tag tag_b = get_tag(entity_b);

		// IMPORTANT: When shape_a != shape_b, this function is called two times, once for each ordering
		// of the shapes. This means we only need to make at most one switch case for any pair of tags.

		switch (tag_a) {

		case Tag::Arrow: {
			destroy_at_end_of_frame(entity_a);
			apply_damage(entity_b, { .type = DamageType::Projectile, .amount = 1 });
		} break;

		case Tag::BladeTrap: {
			on_blade_trap_begin_touch(entity_a, entity_b);
		} break;

		}

		switch (PAIR(tag_a, tag_b)) {

		case PAIR(Tag::Player, Tag::Portal): {
			activate_portal(entity_b);
		} break;

		case PAIR(Tag::Player, Tag::PushableBlock): {
			on_player_begin_touch_pushable_block(entity_a, entity_b);
		} break;

		case PAIR(Tag::Player, Tag::Slime): {
			apply_damage_to_player(entity_a, { .type = DamageType::Melee, .amount = 1 });
		} break;

		}
	}

	void process_contact_end_touch_event(b2ShapeId shape_a, b2ShapeId shape_b)
	{
		const b2BodyId body_a = b2Shape_GetBody(shape_a);
		const b2BodyId body_b = b2Shape_GetBody(shape_b);
		const entt::entity entity_a = (entt::entity)(uintptr_t)b2Body_GetUserData(body_a);
		const entt::entity entity_b = (entt::entity)(uintptr_t)b2Body_GetUserData(body_b);
		const Tag tag_a = get_tag(entity_a);
		const Tag tag_b = get_tag(entity_b);

		// IMPORTANT: When shape_a != shape_b, this function is called two times, once for each ordering
		// of the shapes. This means we only need to make at most one switch case for any pair of tags.

		switch (PAIR(tag_a, tag_b)) {

		case PAIR(Tag::Player, Tag::PushableBlock): {
			on_player_end_touch_pushable_block(entity_a, entity_b);
		} break;

		}
	}
}

