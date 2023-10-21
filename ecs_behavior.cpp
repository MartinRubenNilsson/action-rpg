#include "ecs_behavior.h"
#include "behavior.h"
#include "ecs_behavior_nodes.h"

namespace ecs
{
	extern entt::registry _registry;

	NodePtr _create_enemy_behavior(entt::entity entity)
	{
		return
			create_selector_node({
				create_is_player_in_range_node(entity, 7.f,
					create_selector_node({
						create_is_player_in_range_node(entity, 1.f,
							create_console_execute_node("map reset")),
						create_approach_player_node(entity, 3.f)})),
				create_stop_moving_node(entity)});
	}

	bool emplace_behavior(entt::entity entity, const std::string& behavior_name)
	{
		NodePtr node;

		if (behavior_name == "enemy")
			node = _create_enemy_behavior(entity);

		if (!node) return false;
		_registry.emplace_or_replace<NodePtr>(entity, node);
		return true;
	}

	void update_behaviors(float dt)
	{
		for (auto [entity, node] : _registry.view<NodePtr>().each())
			node->update(dt);
	}
}
