#pragma once

namespace ecs
{
	enum class AiActionType
	{
		None,
		Wait,
		MoveTo,
		Flee,
		Pursue,

		//Some potential examples of other ai actions:
		//Patrol,
		//PlaySound,
	};

	enum class AiActionStatus
	{
		Running,
		Succeeded,
		Failed,
	};

	struct AiAction
	{
		AiActionType type = AiActionType::None;
		AiActionStatus status = AiActionStatus::Running;

		// Add more parameters here as needed. Note that different actions
		// are allowed to use the same parameters for different purposes.
		// You don't need to have unique parameters for each action type,
		// and most actions only use a handful of parameters anyway.

		sf::Vector2f target_position;
		//entt::entity target_entity = entt::null;
		float speed = 0.f;
		float acceptance_radius = 0.1f;
		float duration; // Only used for Wait action as of yet
		float elapsed_time; // Time elapsed since the action started
		float flee_radius;
		entt::entity targetEntity;
	};

	void update_ai_actions(float dt);

	// Add more ai actions here:

	void ai_wait(entt::entity entity, float duration);
	void ai_flee(entt::entity entity, entt::entity targetEntity, float speed, float flee_radius);
	void ai_pursue(entt::entity entity, entt::entity targetEntity, float speed, float pursue_radius);
	void ai_move_to(entt::entity entity, sf::Vector2f target_position, float speed, float acceptance_radius = 0.f);
}
