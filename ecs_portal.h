#pragma once

namespace ecs
{
	// Portals are used to teleport entities between maps.
	// They are defined in Tiled as objects with the "portal" class.

	struct Portal
	{
		std::string target_map; // The name of the map to teleport to.
		bool used = false; // Set to true when the portal have been used, so it can't be used again.
	};

	void update_portals(float dt);

	Portal& emplace_portal(entt::entity entity, const Portal& portal = {});
	bool remove_portal(entt::entity entity);

	bool use_portal(entt::entity entity);
}