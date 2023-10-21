#pragma once

namespace ecs
{
	bool emplace_behavior(entt::entity entity, const std::string& behavior_name);
	void update_behaviors(float dt);
}

