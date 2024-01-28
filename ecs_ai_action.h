#pragma once

namespace ecs
{
	enum class AiActionType
	{
		None, // Does nothing and succeeds immediately. Useful for testing.
		Wait, // Wait for a certain amount of time. Succeeds after the time has elapsed.
		MoveTo, // Move to a certain position. Succeeds when the entity is close enough to the position.
		Pursue, // Pursue a certain entity. Succeeds when the entity is close enough to the target.
		Flee, // Flee from a certain entity. Succeeds when the entity is far enough from the target.
		//Wander, // Wander around randomly. Keeps running, never succeeds or fails.
		//PlayAnimation, // Play a certain animation. Succeeds when it's finished, fails if it doesn't exist.
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
	//void ai_wander(entt::entity entity, float speed);
	//void ai_play_animation(entt::entity entity, const std::string& animation_name);
}
