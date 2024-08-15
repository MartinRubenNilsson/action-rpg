#include "stdafx.h"
#include "ecs_blade_trap.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_common.h"

namespace ecs
{
	constexpr float _BLADE_TRAP_EXTEND_SPEED = 16.f * 6.f; // 6 tiles per second
	constexpr float _BLADE_TRAP_RETRACT_SPEED = 16.f * 2.f; // 2 tiles per second

	extern entt::registry _registry;

	void update_blade_traps(float dt)
	{
		for (auto [entity, blade_trap, body] : _registry.view<BladeTrap, b2BodyId>().each()) {
			blade_trap.update_count++;

			switch (blade_trap.state) {
			case BladeTrapState::Idle: {

				Vector2f direction = { 0.f, 0.f };
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
				if (get_class(hit.entity) != "player") break;

				b2Body_SetLinearVelocity(body, direction * _BLADE_TRAP_EXTEND_SPEED);
				blade_trap.state = BladeTrapState::Extending;

				// TODO: play sound

			} break;
			case BladeTrapState::Retracting: {

				const float distance_to_start = length(blade_trap.start_position - b2Body_GetPosition(body));
				if (distance_to_start >= 0.1f) break;

				b2Body_SetTransform(body, blade_trap.start_position, { 0.f, 0.f });
				b2Body_SetLinearVelocity(body, { 0.f, 0.f });
				blade_trap.state = BladeTrapState::Idle;

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
		
		b2BodyId body = get_body(entity);
		if (B2_IS_NULL(body)) return;
		const Vector2f direction = normalize(blade_trap->start_position - b2Body_GetPosition(body));
		b2Body_SetLinearVelocity(body, direction * _BLADE_TRAP_RETRACT_SPEED);
	}
}