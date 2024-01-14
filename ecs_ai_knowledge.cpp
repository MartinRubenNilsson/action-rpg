#include "stdafx.h"
#include "ecs_ai_knowledge.h"
#include "ecs_tiled.h"
#include "physics_helpers.h"

namespace ecs
{
	extern entt::registry _registry;

	AiEntityInfo get_ai_entity_info(entt::entity entity)
	{
		AiEntityInfo info;
		if (!_registry.valid(entity)) return info;
		info.entity = entity;
		info.name = get_name(entity);
		info.class_ = get_class(entity);
		if (_registry.all_of<b2Body*>(entity)) {
			b2Body* body = _registry.get<b2Body*>(entity);
			info.position = get_world_center(body);
			info.velocity = get_linear_velocity(body);
		}
		return info;
	}

	void update_ai_knowledge(float dt)
	{
		AiEntityInfo player = get_ai_entity_info(find_entity_by_class("player"));

		for (auto [entity, knowledge] : _registry.view<AiKnowledge>().each()) {
			knowledge.me = get_ai_entity_info(entity);
			knowledge.player = player;
		}
	}
}

