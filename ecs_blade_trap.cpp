#include "stdafx.h"
#include "ecs_blade_trap.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_common.h"

namespace ecs
{
	extern entt::registry _registry;

	void update_blade_traps(float dt)
	{
		for (auto [entity, blade_trap, body] : _registry.view<BladeTrap, b2Body*>().each()) {
			blade_trap.update_count++;
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
			if (raycast(ray_start, ray_end, CM_Default, &hit) && get_class(hit.entity) == "player") {
				body->SetLinearVelocity(direction * 16.f);
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
}