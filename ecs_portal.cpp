#include "stdafx.h"
#include "ecs_portal.h"
#include "map.h"

namespace ecs
{
	extern entt::registry _registry;

	void update_portals(float dt)
	{
	}

	Portal& emplace_portal(entt::entity entity, const Portal& portal) {
		return _registry.emplace_or_replace<Portal>(entity, portal);
	}

	bool remove_portal(entt::entity entity) {
		return _registry.remove<Portal>(entity);
	}

	bool use_portal(entt::entity entity)
	{
		Portal* portal = _registry.try_get<Portal>(entity);
		if (!portal) return false;
		portal->used = true;
		return map::open(portal->target_map);
	}
}
