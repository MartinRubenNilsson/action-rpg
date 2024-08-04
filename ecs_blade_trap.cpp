#include "stdafx.h"
#include "ecs_blade_trap.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_common.h"

namespace ecs
{
	constexpr float _BLADE_TRAP_EXTEND_SPEED = 16.f * 6.f; // 6 tiles per second
	constexpr float _BLADE_TRAP_RETRACT_SPEED = 16.f * 2.f; // 6 tiles per second

	extern entt::registry _registry;

	void update_blade_traps(float dt)
	{
		for (auto [entity, blade_trap, body] : _registry.view<BladeTrap, b2Body*>().each()) {
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
				const Vector2f ray_start = body->GetPosition();
				const Vector2f ray_end = ray_start + direction * 16.f * 10.f; // raycast 10 tiles
				RaycastHit hit{};
				if (!raycast(ray_start, ray_end, CM_Default, &hit)) break;
				if (get_class(hit.entity) != "player") break;

				body->SetLinearVelocity(direction * _BLADE_TRAP_EXTEND_SPEED);
				blade_trap.state = BladeTrapState::Extending;

			} break;
			case BladeTrapState::Extending: {

				// TODO

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
		
		b2Body* body = get_body(entity);
		if (!body) return;
		body->SetLinearVelocity({ 0.f, 0.f });
	}
}