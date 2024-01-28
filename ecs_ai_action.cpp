#include "stdafx.h"
#include "ecs_ai_action.h"
#include "physics_helpers.h"

namespace ecs
{
	extern entt::registry _registry;

	void update_ai_actions(float dt)
	{
		// IMPORTANT:
		// Do not include AiKnowledge, AiWorld or AiType in the view below.
		// The AiActions need to be decoupled from these, since we want to be
		// able to trigger AiActions from other systems as well.
		// Hence they need to run independently from the rest of the AI system.

		for (auto [entity, action, body] : _registry.view<AiAction, b2Body*>().each()) {
			action.elapsed_time += dt;
			if (action.status != AiActionStatus::Running) continue;

			const sf::Vector2f position = get_world_center(body);
			set_linear_velocity(body, sf::Vector2f()); // Stop moving by default

			switch (action.type) {
			case AiActionType::None: {
				action.status = AiActionStatus::Succeeded;
				break;
			}
			case AiActionType::MoveTo: {
				sf::Vector2f direction = action.target_position - position;
				float distance = length(direction);
				if (distance > action.acceptance_radius) {
					direction /= distance;
					set_linear_velocity(body, direction * action.speed);
					action.status = AiActionStatus::Running;
				} else {
					action.status = AiActionStatus::Succeeded;
				}
				break;
			}
			case AiActionType::Wait: {
				if (action.elapsed_time < action.duration) {
					action.status = AiActionStatus::Running;
				} else {
					action.status = AiActionStatus::Succeeded;
				}
				break;
			}
			case AiActionType::Pursue: {
				// Get the target's position
				const sf::Vector2f target_position = get_world_center(_registry.get<b2Body*>(action.targetEntity));
				sf::Vector2f direction = target_position - position;
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
			case AiActionType::Flee: {
				// Get the target's position
				const sf::Vector2f target_position = get_world_center(_registry.get<b2Body*>(action.targetEntity));
				sf::Vector2f direction = position - target_position;
				float distance = length(direction);
				if (distance < action.acceptance_radius) {
					direction /= distance;
					set_linear_velocity(body, direction * action.speed);
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
		_set_ai_action(entity, action);
	}

	void ai_flee(entt::entity entity, entt::entity targetEntity, float speed, float acceptance_radius) {
		AiAction action{};
		action.type = AiActionType::Flee;
		action.targetEntity = targetEntity;
		action.speed = speed;
		action.acceptance_radius = acceptance_radius;
		_set_ai_action(entity, action);
	}

	void ai_pursue(entt::entity entity, entt::entity targetEntity, float speed, float acceptance_radius) {
		AiAction action{};
		action.type = AiActionType::Pursue;
		action.targetEntity = targetEntity;
		action.speed = speed;
		action.acceptance_radius = acceptance_radius;
		_set_ai_action(entity, action);
	}

	void ai_move_to(entt::entity entity, sf::Vector2f target_position, float speed, float acceptance_radius)
	{
		AiAction action{};
		action.type = AiActionType::MoveTo;
		action.target_position = target_position;
		action.speed = speed;
		action.acceptance_radius = acceptance_radius;
		_set_ai_action(entity, action);
	}
}