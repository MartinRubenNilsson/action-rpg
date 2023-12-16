#pragma once

namespace ecs
{
	struct AIKnowledge
	{
		entt::entity my_entity = entt::null;
		sf::Vector2f my_position;
		sf::Vector2f my_velocity;
		float my_speed = 0.f;
	};

	void update_ai_knowledge(float dt);
}
