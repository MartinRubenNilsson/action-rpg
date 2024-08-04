#pragma once

namespace ecs
{
	enum class BladeTrapState
	{
		Idle,
		Extending,
		Retracting,
	};

	struct BladeTrap
	{
		unsigned int update_count = 0;
		BladeTrapState state = BladeTrapState::Idle;
		Vector2f start_position;
	};

	void update_blade_traps(float dt);

	BladeTrap& emplace_blade_trap(entt::entity entity);
	BladeTrap* get_blade_trap(entt::entity entity);

	void retract_blade_trap(entt::entity entity);
}
