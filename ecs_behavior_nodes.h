#pragma once
#include "behavior.h"

namespace ecs
{
	using namespace behavior;

	// DECORATORS

	NodePtr create_is_player_in_range_node(entt::entity entity, float range, NodePtr child);

	// LEAVES

	NodePtr create_destroy_entity_node(entt::entity entity);
	NodePtr create_approach_player_node(entt::entity entity);
	NodePtr create_hurt_player_node(int health_to_remove);
	NodePtr create_stop_moving_node(entt::entity entity);
}
