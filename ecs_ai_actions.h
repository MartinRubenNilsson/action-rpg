#pragma once

namespace ecs
{
	struct AIActionMoveToPlayer
	{
		float speed = 0.f;
	};

	void emplace_ai_action(entt::entity entity, const AIActionMoveToPlayer& action);

	void update_ai_actions(float dt);
}
