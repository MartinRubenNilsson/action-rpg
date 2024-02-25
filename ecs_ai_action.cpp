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

		for (auto [entity, action] : _registry.view<AiAction>().each()) {
			if (action.status == AiActionStatus::Running)
				action.running_time += dt;
		}

		for (auto [entity, action, body] : _registry.view<AiAction, b2Body*>().each()) {
			if (action.status != AiActionStatus::Running) continue;

			const sf::Vector2f my_pos = get_world_center(body);
			const sf::Vector2f my_old_vel = get_linear_velocity(body);
			sf::Vector2f my_new_vel;

			switch (action.type) {
			case AiActionType::None: {
				action.status = AiActionStatus::Succeeded;
			} break;
			case AiActionType::Wait: {
				if (action.running_time >= action.duration)
					action.status = AiActionStatus::Succeeded;
			} break;
			case AiActionType::MoveTo: {
				sf::Vector2f to_target = action.position - my_pos;
				float dist = length(to_target);
				if (dist <= action.radius) {
					action.status = AiActionStatus::Succeeded;
				} else {
					my_new_vel = (to_target / dist) * action.speed;
				}
			} break;
			case AiActionType::Pursue: {
				if (!_registry.all_of<b2Body*>(action.entity)) {
					action.status = AiActionStatus::Failed;
					break;
				}
				sf::Vector2f target_pos = get_world_center(_registry.get<b2Body*>(action.entity));
				sf::Vector2f to_target = target_pos - my_pos;
				float dist = length(to_target);
				if (dist <= action.radius) {
					action.status = AiActionStatus::Succeeded;
				} else {
					my_new_vel = (to_target / dist) * action.speed;
				}
			} break;
			case AiActionType::Flee: {
				if (!_registry.all_of<b2Body*>(action.entity)) {
					action.status = AiActionStatus::Failed;
					break;
				}
				sf::Vector2f danger_pos = get_world_center(_registry.get<b2Body*>(action.entity));
				sf::Vector2f to_danger = danger_pos - my_pos;
				float dist = length(to_danger);
				if (dist >= action.radius) {
					action.status = AiActionStatus::Succeeded;
				} else {
					my_new_vel = -(to_danger / dist) * action.speed; // Note the minus sign.
				}
			} break;
			case AiActionType::Wander: {
				//TODO
			} break;
			}

			set_linear_velocity(body, my_new_vel);
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

	void ai_move_to(entt::entity entity, sf::Vector2f target_position, float speed, float acceptance_radius)
	{
		AiAction action{};
		action.type = AiActionType::MoveTo;
		action.position = target_position;
		action.speed = speed;
		action.radius = acceptance_radius;
		_set_ai_action(entity, action);
	}

	void ai_pursue(entt::entity entity, entt::entity target_entity, float speed, float pursue_radius)
	{
		AiAction action{};
		action.type = AiActionType::Pursue;
		action.entity = target_entity;
		action.speed = speed;
		action.radius = pursue_radius;
		_set_ai_action(entity, action);
	}

	void ai_flee(entt::entity entity, entt::entity target_entity, float speed, float flee_radius)
	{
		AiAction action{};
		action.type = AiActionType::Flee;
		action.entity = target_entity;
		action.speed = speed;
		action.radius = flee_radius;
		_set_ai_action(entity, action);
	}
}