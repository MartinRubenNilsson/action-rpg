#pragma once

namespace ecs
{
	struct AiEntityInfo
	{
		entt::entity entity = entt::null;
		std::string name;
		std::string class_;
		sf::Vector2f current_position;
		sf::Vector2f current_velocity;
		float speed = 0.f;
	};

	struct AiWorld
	{
		AiEntityInfo player;
		std::vector<AiEntityInfo> ais;
	};

	struct AiKnowledge
	{
		AiEntityInfo me;
	};

	void update_ai_knowledge(float dt);

	const AiWorld& get_ai_world();

	void emplace_ai_knowledge(entt::entity entity);
}
