#pragma once

namespace ecs
{
	enum class BehaviorType
	{
		Enemy,
	};

	bool emplace_behavior(entt::entity entity, BehaviorType type);
	void update_behaviors(float dt);
}

