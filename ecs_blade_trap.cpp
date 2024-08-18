#include "stdafx.h"
#include "ecs_blade_trap.h"
#include "ecs_sprites.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"

namespace ecs
{
	constexpr float _BLADE_TRAP_EXTEND_SPEED = 16.f * 6.f; // 6 tiles per second
	constexpr float _BLADE_TRAP_RETRACT_SPEED = 16.f * 2.f; // 2 tiles per second

	extern entt::registry _registry;

	void update_blade_traps(float dt)
	{
		for (auto [entity, blade_trap, body] : _registry.view<BladeTrap, b2BodyId>().each()) {

			blade_trap.update_count++;
			blade_trap.state_timer.update(dt);
			if (!blade_trap.state_timer.finished()) continue;

			switch (blade_trap.state) {
			case BladeTrapState::Idle: {

				Vector2f direction;
				switch (blade_trap.update_count % 4) {
				case 0: direction = { 1.f, 0.f }; break;
				case 1: direction = { 0.f, 1.f }; break;
				case 2: direction = { -1.f, 0.f }; break;
				case 3: direction = { 0.f, -1.f }; break;
				}

				const Vector2f ray_start = b2Body_GetPosition(body);
				const Vector2f ray_end = ray_start + direction * 16.f * 10.f; // raycast 10 tiles
				RaycastHit hit{};
				if (!raycast_closest(ray_start, ray_end, CM_Default, &hit)) break;
				if (is_zero(b2Body_GetLinearVelocity(hit.body))) break;

				b2Body_SetLinearVelocity(body, direction * _BLADE_TRAP_EXTEND_SPEED);
				blade_trap.state = BladeTrapState::Extending;

				// TODO: play sound

			} break;
			case BladeTrapState::Retracting: {

				const Vector2f to_start = blade_trap.start_position - b2Body_GetPosition(body);
				const float dist_to_start = length(to_start);

				if (dist_to_start >= 1.f) {
					const Vector2 dir_to_start = to_start / dist_to_start;
					b2Body_SetLinearVelocity(body, dir_to_start * _BLADE_TRAP_RETRACT_SPEED);
					break;
				}

				b2Body_SetTransform(body, blade_trap.start_position, b2Rot_identity);
				b2Body_SetLinearVelocity(body, b2Vec2_zero);
				blade_trap.state = BladeTrapState::Idle;

				blade_trap.state_timer = { 0.2f };
				blade_trap.state_timer.start();
				{
					SpriteShake& shake = emplace_sprite_shake(entity);
					shake.duration = 0.2f;
					shake.magnitude = 4.f;
					shake.exponent = 3.f;
				}

				// TODO: play sound

			} break;
			}
		}
	}

	BladeTrap& emplace_blade_trap(entt::entity entity)
	{
		return _registry.emplace_or_replace<BladeTrap>(entity);
	}

	BladeTrap* get_blade_trap(entt::entity entity)
	{
		return _registry.try_get<BladeTrap>(entity);
	}

	void retract_blade_trap(entt::entity entity)
	{
		BladeTrap* blade_trap = get_blade_trap(entity);
		if (!blade_trap) return;
		if (blade_trap->state != BladeTrapState::Extending) return;
		blade_trap->state = BladeTrapState::Retracting;
		blade_trap->state_timer = { 0.4f };
		blade_trap->state_timer.start();
		{
			SpriteShake& shake = emplace_sprite_shake(entity);
			shake.duration = 0.4f;
			shake.magnitude = 6.f;
			shake.exponent = 3.f;
		}
	}
}