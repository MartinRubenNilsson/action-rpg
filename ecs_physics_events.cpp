#include "stdafx.h"
#include "ecs_physics_events.h"
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
	void process_sensor_begin_touch_event(b2ShapeId sensor_shape, b2ShapeId visitor_shape)
	{
		const b2BodyId sensor_body = b2Shape_GetBody(sensor_shape);
		const b2BodyId visitor_body = b2Shape_GetBody(visitor_shape);
		const entt::entity sensor_entity = (entt::entity)(uintptr_t)b2Body_GetUserData(sensor_body);
		const entt::entity visitor_entity = (entt::entity)(uintptr_t)b2Body_GetUserData(visitor_body);
		const std::string sensor_class = get_class(sensor_entity);
		const std::string visitor_class = get_class(visitor_entity);

		if (sensor_class.empty() && visitor_class.empty()) return;

		if (sensor_class == "pickup") {
			if (visitor_class == "player") {
				on_player_begin_touch_pickup(visitor_entity, sensor_entity);
			}
		}
	}

	void process_sensor_end_touch_event(b2ShapeId sensor_shape, b2ShapeId visitor_shape)
	{
	}

	void process_contact_begin_touch_event(b2ShapeId shape_a, b2ShapeId shape_b)
	{
		b2BodyId body_a = b2Shape_GetBody(shape_a);
		b2BodyId body_b = b2Shape_GetBody(shape_b);
		entt::entity entity_a = (entt::entity)(uintptr_t)b2Body_GetUserData(body_a);
		entt::entity entity_b = (entt::entity)(uintptr_t)b2Body_GetUserData(body_b);
		std::string class_a = get_class(entity_a);
		std::string class_b = get_class(entity_b);
		if (class_a.empty() && class_b.empty()) return;

		// Sort the classes alphabetically; this reduces the number of cases we need to handle.
 		if (class_a.compare(class_b) > 0) {
			std::swap(shape_a, shape_b);
			std::swap(body_a, body_b);
			std::swap(entity_a, entity_b);
			std::swap(class_a, class_b);
		}

		if (class_a.empty()) {
			if (class_b == "arrow") {
				// TODO add audio::play("event:/snd_arrow_hit"); when we have nice animation for arrow hitting a wall
				destroy_at_end_of_frame(entity_b);
			} else if (class_b == "blade_trap") {
				retract_blade_trap(entity_b);
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
		} else if (class_a == "player") {
			if (class_b == "portal") {
				activate_portal(entity_b);
			} else if (class_b == "slime") {
				apply_damage_to_player(entity_a, { .type = DamageType::Melee, .amount = 1 });
			} else if (class_b == "pushable_block") {
				on_player_begin_touch_pushable_block(entity_a);
			}
		}
	}

	void process_contact_end_touch_event(b2ShapeId shape_a, b2ShapeId shape_b)
	{
		b2BodyId body_a = b2Shape_GetBody(shape_a);
		b2BodyId body_b = b2Shape_GetBody(shape_b);
		entt::entity entity_a = (entt::entity)(uintptr_t)b2Body_GetUserData(body_a);
		entt::entity entity_b = (entt::entity)(uintptr_t)b2Body_GetUserData(body_b);
		std::string class_a = get_class(entity_a);
		std::string class_b = get_class(entity_b);
		if (class_a.empty() && class_b.empty()) return;

		// Sort the classes alphabetically; this reduces the number of cases we need to handle.
		if (class_a.compare(class_b) > 0) {
			std::swap(shape_a, shape_b);
			std::swap(body_a, body_b);
			std::swap(entity_a, entity_b);
			std::swap(class_a, class_b);
		}

		if (class_a == "player") {
			if (class_b == "pushable_block") {
				b2Body_SetLinearVelocity(body_b, b2Vec2_zero);
				on_player_end_touch_pushable_block(entity_a);
			}
		}
	}
}

