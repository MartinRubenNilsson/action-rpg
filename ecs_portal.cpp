#include "stdafx.h"
#include "ecs_portal.h"
#include "map.h"
#include "audio.h"

namespace ecs
{
	extern entt::registry _registry;

	void update_portals(float dt)
	{
		//Empty
	}

	entt::entity find_active_portal_entity()
	{
		for (auto [entity, portal] : _registry.view<const Portal>().each()) {
			if (portal.activated) return entity;
		}
		return entt::null;
	}

	Portal& emplace_portal(entt::entity entity, const Portal& portal) {
		return _registry.emplace_or_replace<Portal>(entity, portal);
	}

	Portal& get_portal(entt::entity entity) {
		return _registry.get<Portal>(entity);
	}

	Portal* try_get_portal(entt::entity entity) {
		return _registry.try_get<Portal>(entity);
	}

	bool remove_portal(entt::entity entity) {
		return _registry.remove<Portal>(entity);
	}

	bool has_portal(entt::entity entity) {
		return _registry.all_of<Portal>(entity);
	}

	bool activate_portal(entt::entity entity)
	{
		if (!_registry.all_of<Portal>(entity)) return false;
		Portal& portal = _registry.get<Portal>(entity);
		if (portal.activated) return false;
		portal.activated = true;
		if (map::open(portal.target_map)) {
			audio::play("event:/snd_map_transition");
		}
		return false;
	}
}
