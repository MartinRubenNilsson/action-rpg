#pragma once
#include "timer.h"

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
		Timer state_timer; // for pausing between state changes
		Vector2f start_position;
	};

	void update_blade_traps(float dt);

	BladeTrap& emplace_blade_trap(entt::entity entity);
	BladeTrap* get_blade_trap(entt::entity entity);

	void on_blade_trap_begin_touch(entt::entity blade_trap_entity, entt::entity other_entity);
}
