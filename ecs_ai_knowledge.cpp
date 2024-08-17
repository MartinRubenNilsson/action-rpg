#include "stdafx.h"
#include "ecs_ai_knowledge.h"
#include "ecs_common.h"
#include "ecs_physics.h"

namespace ecs
{
	extern entt::registry _registry;
	AiWorld _ai_world;

	AiEntityInfo _get_ai_entity_info(entt::entity entity)
	{
		if (!_registry.valid(entity)) return AiEntityInfo();
		AiEntityInfo info{};
		info.entity = entity;
		info.name = get_name(entity);
		info.class_ = get_class(entity);
		if (b2BodyId body = get_body(entity); B2_IS_NON_NULL(body)) {
			info.position = b2Body_GetPosition(body);
			info.velocity = b2Body_GetLinearVelocity(body);
		}
		get_float(entity, "speed", info.p_speed);
		return info;
	} 

	void update_ai_knowledge_and_world(float dt)
	{
		_ai_world.player = _get_ai_entity_info(find_entity_by_class(Class::Player));
		_ai_world.ais.clear();

		for (auto [entity, knowledge] : _registry.view<AiKnowledge>().each()) {
			knowledge.me = _get_ai_entity_info(entity);
			_ai_world.ais.push_back(knowledge.me);
		}
	}

	const AiWorld& get_ai_world()
	{
		return _ai_world;
	}

	AiKnowledge& emplace_ai_knowledge(entt::entity entity)
	{
		AiKnowledge& knowledge = _registry.emplace_or_replace<AiKnowledge>(entity);
		if (b2BodyId body = get_body(entity); B2_IS_NON_NULL(body)) {
			knowledge.initial_position = b2Body_GetPosition(body);
			knowledge.initial_velocity = b2Body_GetLinearVelocity(body);
		}
		return knowledge;
	}
}

