#pragma once

namespace ecs
{
	enum class AiActionType
	{
		None,
		MoveToPosition,
	};

	struct AiAction
	{
		AiActionType type = AiActionType::None;
		sf::Vector2f position;
		float speed = 0.f;
	};

	void update_ai_actions(float dt);

	void emplace_ai_action(entt::entity entity, const AiAction& action);
}
