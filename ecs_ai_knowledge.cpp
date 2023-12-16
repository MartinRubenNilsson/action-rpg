#include "stdafx.h"
#include "ecs_ai_knowledge.h"
#include "physics_helpers.h"

namespace ecs
{
	extern entt::registry _registry;

	void update_ai_knowledge(float dt)
	{
		for (auto [entity, knowledge] : _registry.view<AIKnowledge>().each()) {
			knowledge = {};
			knowledge.my_entity = entity;
		}

		for (auto [entity, knowledge, body] : _registry.view<AIKnowledge, b2Body*>().each()) {
			knowledge.my_position = get_world_center(body);
			knowledge.my_velocity = get_linear_velocity(body);
			knowledge.my_speed = length(knowledge.my_velocity);
		}
	}
}

