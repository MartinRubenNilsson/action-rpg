#include "stdafx.h"
#include "ecs_portal.h"
#include "map.h"

namespace ecs
{
	extern entt::registry _registry;

	void update_portals(float dt)
	{
		//Empty
	}

	Portal& emplace_portal(entt::entity entity, const Portal& portal) {
		return _registry.emplace_or_replace<Portal>(entity, portal);
	}

	bool remove_portal(entt::entity entity) {
		return _registry.remove<Portal>(entity);
	}

	bool activate_portal(entt::entity entity)
	{
		if (!_registry.all_of<Portal>(entity)) return false;
		Portal& portal = _registry.get<Portal>(entity);
		if (portal.activated) return false;
		portal.activated = true;
		return map::open(portal.target_map);
	}
}
