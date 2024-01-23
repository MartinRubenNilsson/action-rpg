#include "stdafx.h"
#include "ecs_ai_action.h"
#include "physics_helpers.h"

namespace ecs
{
	extern entt::registry _registry;

	void update_ai_actions(float dt)
	{
		for (auto [entity, action, body] : _registry.view<AiAction, b2Body*>().each()) {
			switch (action.type) {
				case AiActionType::MoveToPosition: {
					sf::Vector2f direction = action.target_position - get_world_center(body);
					float distance = length(direction);
					if (distance < 0.1f) continue;
					direction /= distance;
					set_linear_velocity(body, direction * action.speed);
					break;
				}
				case AiActionType::MoveToEntity: {
					if (!_registry.all_of<b2Body*>(action.target_entity)) continue;
					b2Body* target_body = _registry.get<b2Body*>(action.target_entity);
					sf::Vector2f direction = get_world_center(target_body) - get_world_center(body);
					float distance = length(direction);
					if (distance < 0.1f) continue;
					direction /= distance;
					set_linear_velocity(body, direction * action.speed);
					break;
				}
			}
		}
	}

	void emplace_ai_action(entt::entity entity, const AiAction& action) {
		_registry.emplace_or_replace<AiAction>(entity, action);
	}
}