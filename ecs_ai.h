#pragma once

namespace ecs
{
	enum class AiType
	{
		Slime,

		// Add new ai types here. Examples:
		//Spider,
		//NPC,
		//Boss,
	};

	void update_ai(float dt);

	void emplace_ai(entt::entity entity, AiType type);
}