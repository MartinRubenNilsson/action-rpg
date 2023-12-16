#include "stdafx.h"
#include "ecs_ai_actions.h"
#include "ecs_player.h"
#include "physics_helpers.h"

namespace ecs
{
	extern entt::registry _registry;

	void emplace_ai_action(entt::entity entity, const AIActionMoveToPlayer& action) {
		_registry.emplace_or_replace<AIActionMoveToPlayer>(entity, action);
	}

	void update_ai_actions(float dt)
	{
		if (player_exists()) {
			sf::Vector2f player_world_center = get_player_world_center();
			for (auto [entity, action, body] : _registry.view<AIActionMoveToPlayer, b2Body*>().each()) {
				sf::Vector2f direction = player_world_center - get_world_center(body);
				float distance = length(direction);
				if (distance < 0.1f) continue;
				direction /= distance;
				set_linear_velocity(body, direction * action.speed);
			}
		}
	}
}