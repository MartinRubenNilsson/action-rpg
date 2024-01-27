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
			case AiActionType::None: {
				action.status = AiActionStatus::Succeeded;
				break;
			}
			case AiActionType::MoveToPosition: {
				sf::Vector2f direction = action.target_position - get_world_center(body);
				float distance = length(direction);

				if (distance > action.acceptance_radius) {
					direction /= distance;
					set_linear_velocity(body, direction * action.speed);
					action.status = AiActionStatus::Running;
				}
				else {
					action.status = AiActionStatus::Succeeded;
				}
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
				case AiActionType::Wait: {
					if (action.elapsedTime < action.duration) {
						action.elapsedTime += dt;
						action.status = AiActionStatus::Running;
					}
					else {
						action.status = AiActionStatus::Succeeded;
					}
					break;
				}
			}
		}
	}

	void _set_ai_action(entt::entity entity, const AiAction& action) {
		_registry.emplace_or_replace<AiAction>(entity, action);
	}

	void ai_wait(entt::entity entity, float duration)
	{
		AiAction action{};
		action.type = AiActionType::Wait;
		action.duration = duration;
		action.elapsedTime = 0.f; // Initialize elapsedTime to 0
		_set_ai_action(entity, action);
	}

	void ai_move_to_position(entt::entity entity, sf::Vector2f target_position, float speed)
	{
		AiAction action{};
		action.type = AiActionType::MoveToPosition;
		action.target_position = target_position;
		action.speed = speed;
		_set_ai_action(entity, action);
	}

	void ai_move_to_entity(entt::entity entity, entt::entity target_entity, float speed)
	{
		AiAction action{};
		action.type = AiActionType::MoveToEntity;
		action.target_entity = target_entity;
		action.speed = speed;
		_set_ai_action(entity, action);
	}
}