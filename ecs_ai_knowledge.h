#pragma once

namespace ecs
{
	struct AiEntityInfo
	{
		entt::entity entity = entt::null;
		std::string name;
		std::string class_;
		sf::Vector2f position;
		sf::Vector2f velocity;
	};

	struct AiKnowledge
	{
		AiEntityInfo me;
		AiEntityInfo player;
	};

	AiEntityInfo get_ai_entity_info(entt::entity entity);
	void update_ai_knowledge(float dt);
}
