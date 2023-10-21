#pragma once
#include "behavior.h"

namespace ecs
{
	using namespace behavior;

	NodePtr create_is_player_in_range_node(entt::entity entity, float range, NodePtr child);
	NodePtr create_approach_player_node(entt::entity entity, float speed);
	NodePtr create_stop_moving_node(entt::entity entity);
}
