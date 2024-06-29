#pragma once

namespace ecs
{
	struct AiEntityInfo
	{
		entt::entity entity = entt::null;
		std::string name;
		std::string class_;
		Vector2f position;
		Vector2f velocity;

		// PROPERTIES
		float p_speed = 0.f;
	};

	struct AiWorld
	{
		AiEntityInfo player;
		std::vector<AiEntityInfo> ais;
	};

	struct AiKnowledge
	{
		AiEntityInfo me;
		Vector2f initial_position;
		Vector2f initial_velocity;
	};

	void update_ai_knowledge_and_world(float dt);

	const AiWorld& get_ai_world();
	AiKnowledge& emplace_ai_knowledge(entt::entity entity);
}
