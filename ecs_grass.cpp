#include "stdafx.h"
#include "ecs_grass.h"

namespace ecs
{
	extern entt::registry _registry;

	Grass& emplace_grass(entt::entity entity)
	{
		return _registry.emplace_or_replace<Grass>(entity);
	}

	GrassUniformBlock& emplace_grass_uniform_block(entt::entity entity)
	{
		return _registry.emplace_or_replace<GrassUniformBlock>(entity);
	}
}
