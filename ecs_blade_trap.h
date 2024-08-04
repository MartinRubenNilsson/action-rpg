#pragma once

namespace ecs
{
	enum class BladeTrapState
	{
		Idle,
		Active,
	};

	struct BladeTrap
	{
		unsigned int update_count = 0;
		BladeTrapState state = BladeTrapState::Idle;
	};

	void update_blade_traps(float dt);

	BladeTrap& emplace_blade_trap(entt::entity entity);
	BladeTrap* get_blade_trap(entt::entity entity);
}
