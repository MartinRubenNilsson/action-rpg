#include "stdafx.h"
#include "ecs_ai_action.h"
#include "physics_helpers.h"
#include "random.h"

namespace ecs
{
	extern entt::registry _registry;
	float _ai_action_time = 0.f;

	void update_ai_actions(float dt)
	{
		_ai_action_time += dt;

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
			const sf::Vector2f my_old_dir = normalize(my_old_vel);
			sf::Vector2f my_new_vel;

			switch (action.type) {
			case AiActionType::None: {
				// Do nothing.
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
				if (action.duration > 0.f && action.running_time >= action.duration) {
					action.status = AiActionStatus::Succeeded;
					break;
				}
				if (action.radius <= 0.f) {
					action.status = AiActionStatus::Failed;
					break;
				}
				sf::Vector2f my_new_dir = my_old_dir;
				if (is_zero(my_new_dir))
					my_new_dir = random::on_circle();
				float noise_sample = random::fractal_perlin_noise(
					action.position.x * 0.01f,
					action.position.y * 0.01f,
					_ai_action_time * 1.f);
				my_new_dir = rotate(my_new_dir, 5.f * noise_sample * dt);
				sf::Vector2f to_center = action.position - my_pos;
				float dist = length(to_center);
				if (dist > action.radius * 0.5f) {
					float lerp_t = std::clamp((dist / action.radius - 0.5f) * 2.f, 0.f, 1.f);
					my_new_dir = lerp_polar(my_new_dir, to_center / dist, lerp_t);
				}
				my_new_vel = my_new_dir * action.speed;
			} break;
			}

			set_linear_velocity(body, my_new_vel);
		}
	}

	void _set_ai_action(entt::entity entity, const AiAction& action) {
		_registry.emplace_or_replace<AiAction>(entity, action);
	}

	void ai_none(entt::entity entity)
	{
		AiAction action{};
		action.type = AiActionType::None;
		_set_ai_action(entity, action);
	}

	void ai_wait(entt::entity entity, float duration)
	{
		AiAction action{};
		action.type = AiActionType::Wait;
		action.duration = duration;
		_set_ai_action(entity, action);
	}

	void ai_move_to(entt::entity entity, const sf::Vector2f& target_position, float speed, float acceptance_radius)
	{
		AiAction action{};
		action.type = AiActionType::MoveTo;
		action.position = target_position;
		action.speed = speed;
		action.radius = acceptance_radius;
		_set_ai_action(entity, action);
	}

	void ai_pursue(entt::entity entity, entt::entity target_entity, float speed, float acceptance_radius)
	{
		AiAction action{};
		action.type = AiActionType::Pursue;
		action.entity = target_entity;
		action.speed = speed;
		action.radius = acceptance_radius;
		_set_ai_action(entity, action);
	}

	void ai_flee(entt::entity entity, entt::entity target_entity, float speed, float acceptance_radius)
	{
		AiAction action{};
		action.type = AiActionType::Flee;
		action.entity = target_entity;
		action.speed = speed;
		action.radius = acceptance_radius;
		_set_ai_action(entity, action);
	}

	void ai_wander(entt::entity entity, const sf::Vector2f& wander_center, float speed, float wander_radius, float duration)
	{
		AiAction action{};
		action.type = AiActionType::Wander;
		action.position = wander_center;
		action.speed = speed;
		action.radius = wander_radius;
		action.duration = duration;
		_set_ai_action(entity, action);
	}
}