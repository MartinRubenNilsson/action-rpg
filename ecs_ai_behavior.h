#pragma once

namespace ecs
{
	enum class AiBehaviorType
	{
		None,
		Slime,
	};

	struct AiBehavior
	{
		AiBehaviorType type = AiBehaviorType::None;
	};

	void update_ai_behaviors(float dt);
}