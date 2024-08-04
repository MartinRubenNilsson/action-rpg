#pragma once

namespace ecs
{
	struct BladeTrap
	{
		// TODO: Implement
		int t;
	};

	void update_blade_traps(float dt);

	BladeTrap& emplace_blade_trap(entt::entity entity);
	BladeTrap* get_blade_trap(entt::entity entity);
}
