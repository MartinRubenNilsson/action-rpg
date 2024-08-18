#include "stdafx.h"
#include "ecs_physics_events.h"
#include "ecs_common.h"
#include "ecs_sprite.h"
#include "ecs_player.h"
#include "ecs_pickups.h"
#include "ecs_damage.h"
#include "ecs_portal.h"
#include "ecs_blade_trap.h"
#include "audio.h"
#include "console.h"

#define PAIR(a, b) ((uint64_t)a << 32 | (uint64_t)b)
#define BOTH_CASES(a, b) case PAIR(b, a): swap(); [[fallthrough]]; case PAIR(a, b):

namespace ecs
{
	void process_sensor_begin_touch_event(b2ShapeId sensor_shape, b2ShapeId visitor_shape)
	{
		const b2BodyId sensor_body = b2Shape_GetBody(sensor_shape);
		const b2BodyId visitor_body = b2Shape_GetBody(visitor_shape);
		const entt::entity sensor_entity = (entt::entity)(uintptr_t)b2Body_GetUserData(sensor_body);
		const entt::entity visitor_entity = (entt::entity)(uintptr_t)b2Body_GetUserData(visitor_body);
		const Tag sensor_tag = get_tag(sensor_entity);
		const Tag visitor_tag = get_tag(visitor_entity);

		switch (PAIR(sensor_tag, visitor_tag)) {

		case PAIR(Tag::Pickup, Tag::Player): {
			on_player_begin_touch_pickup(visitor_entity, sensor_entity);
		} break;

		}
	}

	void process_sensor_end_touch_event(b2ShapeId sensor_shape, b2ShapeId visitor_shape)
	{
		const b2BodyId sensor_body = b2Shape_GetBody(sensor_shape);
		const b2BodyId visitor_body = b2Shape_GetBody(visitor_shape);
		const entt::entity sensor_entity = (entt::entity)(uintptr_t)b2Body_GetUserData(sensor_body);
		const entt::entity visitor_entity = (entt::entity)(uintptr_t)b2Body_GetUserData(visitor_body);
		const Tag sensor_tag = get_tag(sensor_entity);
		const Tag visitor_tag = get_tag(visitor_entity);

		//TODO
	}

	void process_contact_begin_touch_event(b2ShapeId shape_a, b2ShapeId shape_b)
	{
		b2BodyId body_a = b2Shape_GetBody(shape_a);
		b2BodyId body_b = b2Shape_GetBody(shape_b);
		entt::entity entity_a = (entt::entity)(uintptr_t)b2Body_GetUserData(body_a);
		entt::entity entity_b = (entt::entity)(uintptr_t)b2Body_GetUserData(body_b);
		Tag tag_a = get_tag(entity_a);
		Tag tag_b = get_tag(entity_b);

		const auto swap = [&]() {
			std::swap(shape_a, shape_b);
			std::swap(body_a, body_b);
			std::swap(entity_a, entity_b);
			std::swap(tag_a, tag_b);
		};

		switch (PAIR(tag_a, tag_b)) {

		BOTH_CASES(Tag::Player, Tag::Portal) {
			activate_portal(entity_b);
		} break;

		BOTH_CASES(Tag::Player, Tag::PushableBlock) {
			on_player_begin_touch_pushable_block(entity_a);
		} break;

		BOTH_CASES(Tag::Player, Tag::Slime) {
			apply_damage_to_player(entity_a, { .type = DamageType::Melee, .amount = 1 });
		} break;

		BOTH_CASES(Tag::Player, Tag::BladeTrap) {
			apply_damage_to_player(entity_a, { .amount = 1 });
		} break;

		BOTH_CASES(Tag::Arrow, Tag::None) {
			destroy_at_end_of_frame(entity_a);
		} break;

		BOTH_CASES(Tag::Arrow, Tag::Slime) {
			destroy_at_end_of_frame(entity_a);
			apply_damage(entity_b, { .type = DamageType::Projectile, .amount = 1 });
		} break;

		BOTH_CASES(Tag::Arrow, Tag::Bomb) {
			destroy_at_end_of_frame(entity_a);
			apply_damage(entity_b, { .type = DamageType::Projectile, .amount = 1 });
		} break;

		BOTH_CASES(Tag::BladeTrap, Tag::None) {
			retract_blade_trap(entity_a);
			SpriteShake& shake = emplace_sprite_shake(entity_a);
			shake.duration = 0.2f;
			shake.magnitude = 10.f;
		} break;

		}
	}

	void process_contact_end_touch_event(b2ShapeId shape_a, b2ShapeId shape_b)
	{
		b2BodyId body_a = b2Shape_GetBody(shape_a);
		b2BodyId body_b = b2Shape_GetBody(shape_b);
		entt::entity entity_a = (entt::entity)(uintptr_t)b2Body_GetUserData(body_a);
		entt::entity entity_b = (entt::entity)(uintptr_t)b2Body_GetUserData(body_b);
		Tag tag_a = get_tag(entity_a);
		Tag tag_b = get_tag(entity_b);

		const auto swap = [&]() {
			std::swap(shape_a, shape_b);
			std::swap(body_a, body_b);
			std::swap(entity_a, entity_b);
			std::swap(tag_a, tag_b);
		};

		switch (PAIR(tag_a, tag_b)) {

		BOTH_CASES(Tag::Player, Tag::PushableBlock) {
			on_player_end_touch_pushable_block(entity_a);
			b2Body_SetLinearVelocity(body_b, b2Vec2_zero);
		} break;

		}
	}
}

