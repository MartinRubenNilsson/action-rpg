#pragma once

namespace ecs
{
	enum class AiActionType
	{
		None, // Do nothing. Runs forever.
		Wait, // Wait for a certain amount of time. Succeeds after the time has elapsed.
		MoveTo, // Move to a certain position. Succeeds when the entity is sufficiently close.
		Pursue, // Pursue a certain entity. Succeeds when the entity is sufficiently close.
		Flee, // Flee from a certain entity. Succeeds when the entity is sufficiently far away.
		Wander, // Wander around randomly. Runs forever.
		//PlayAnimation, // Play a certain animation. Succeeds when it's finished, fails if it doesn't exist.
	};

	enum class AiActionStatus
	{
		Running,
		Succeeded,
		Failed,
		TimedOut,
	};

	struct AiAction
	{
		AiActionType type = AiActionType::None;
		AiActionStatus status = AiActionStatus::Running;
		float running_time = 0.f;
		float max_running_time = 0.f; // If this is 0, the action runs forever.

		// ACTION-SPECIFIC PARAMETERS

		entt::entity entity = entt::null;
		sf::Vector2f position;
		float speed = 0.f;
		float radius = 0.f;
		float duration = 0.f;
	};

	void update_ai_actions(float dt);

	bool ai_set_max_running_time(entt::entity entity, float max_running_time);

	// ACTIONS

	void ai_none(entt::entity entity);
	void ai_wait(entt::entity entity, float duration);
	void ai_move_to(entt::entity entity, const sf::Vector2f& target_position, float speed, float acceptance_radius);
	void ai_pursue(entt::entity entity, entt::entity target_entity, float speed, float acceptance_radius);
	void ai_flee(entt::entity entity, entt::entity target_entity, float speed, float acceptance_radius);
	void ai_wander(entt::entity entity, const sf::Vector2f& wander_center, float speed, float wander_radius);
	//void ai_play_animation(entt::entity entity, const std::string& animation_name);
}
