#include "ecs_behavior.h"
#include "behavior.h"
#include "ecs_behavior_nodes.h"

namespace ecs
{
	extern entt::registry _registry;

	bool emplace_behavior(entt::entity entity, BehaviorType type)
	{
		NodePtr root_node;
		switch (type)
		{
		case BehaviorType::Enemy:
			root_node = create_selector_node({
				create_is_player_in_range_node(entity, 7.f,
					create_selector_node({
						create_is_player_in_range_node(entity, 1.f,
							create_hurt_player_node(1)),
						create_approach_player_node(entity)})),
				create_stop_moving_node(entity) });
			break;
		}
		if (!root_node) return false;
		_registry.emplace_or_replace<NodePtr>(entity, root_node);
		return true;
	}

	void update_behaviors(float dt)
	{
		for (auto [entity, node] : _registry.view<NodePtr>().each())
			if (node) node->update(dt);
	}
}
