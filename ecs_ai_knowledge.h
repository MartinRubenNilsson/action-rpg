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
		float speed = 0.f;
		//Add more info here as needed
	};

	struct AiWorld
	{
		AiEntityInfo player;
		std::vector<AiEntityInfo> ais;
		//Add more info here as needed
	};

	struct AiKnowledge
	{
		AiEntityInfo me;
		//Add more info here as needed
	};

	void update_ai_knowledge_and_world(float dt);

	const AiWorld& get_ai_world();
}
