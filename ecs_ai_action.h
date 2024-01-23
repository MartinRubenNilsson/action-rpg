#pragma once

namespace ecs
{
	enum class AiActionType
	{
		None,
		MoveToPosition,
		MoveToEntity,
	};

	struct AiAction
	{
		AiActionType type = AiActionType::None;
		sf::Vector2f target_position;
		entt::entity target_entity = entt::null;
		float speed = 0.f;
	};

	void update_ai_actions(float dt);

	void emplace_ai_action(entt::entity entity, const AiAction& action);
}
