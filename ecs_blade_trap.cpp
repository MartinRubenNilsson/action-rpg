#include "stdafx.h"
#include "ecs_blade_trap.h"
#include "ecs_physics.h"

namespace ecs
{
	extern entt::registry _registry;

	void update_blade_traps(float dt)
	{
		for (auto [entity, blade_trap] : _registry.view<BladeTrap>().each()) {
			// TODO
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