#pragma once

namespace ecs
{
	enum class AiActionType
	{
		None,
		Wait, // Wait for a certain amount of time
		MoveTo, // Move to a certain position
		Pursue, // Pursue a certain entity
		Flee, // Flee from a certain entity

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
		entt::entity target_entity = entt::null;
		float speed = 0.f;
		float radius = 0.f;
		float duration = 0.f;
		float elapsed_time = 0.f;
	};

	void update_ai_actions(float dt);

	// Add more ai actions here:

	void ai_wait(entt::entity entity, float duration);
	void ai_move_to(entt::entity entity, sf::Vector2f target_position, float speed, float acceptance_radius);
	void ai_pursue(entt::entity entity, entt::entity target_entity, float speed, float pursue_radius);
	void ai_flee(entt::entity entity, entt::entity target_entity, float speed, float flee_radius);
}
