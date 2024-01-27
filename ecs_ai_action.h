#pragma once

namespace ecs
{
	enum class AiActionType
	{
		None,
		MoveToPosition,
		MoveToEntity,
		Wait,

		//Some potential examples of other ai actions:
		//Patrol,
		//FleeFromEntity,
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
		entt::entity target_entity = entt::null;
		float speed = 0.f;
		float acceptance_radius = 0.1f;
		float waitTime; // Only used for Wait action
		float elapsedTime; // Time elapsed since the action started
		float duration;
	};

	void update_ai_actions(float dt);
	void ai_wait(entt::entity entity, float duration);

	// Add more ai actions here:

	void ai_move_to_position(entt::entity entity, sf::Vector2f target_position, float speed);
	void ai_move_to_entity(entt::entity entity, entt::entity target_entity, float speed);
}
