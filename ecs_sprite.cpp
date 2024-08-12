#include "stdafx.h"
#include "ecs_sprite.h"

namespace ecs
{
	extern entt::registry _registry;

	sprites::Sprite& emplace_sprite(entt::entity entity)
	{
		return _registry.emplace_or_replace<sprites::Sprite>(entity);
	}

	sprites::Sprite* get_sprite(entt::entity entity)
	{
		return _registry.try_get<sprites::Sprite>(entity);
	}
}