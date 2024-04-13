#pragma once

namespace ecs
{
	// Portals are used to teleport the player between maps.
	// They are defined in Tiled as objects with the "portal" class.

	struct Portal
	{
		std::string target_map;
		sf::Vector2f target_pos;
		bool has_target_pos = false;
		bool activated = false;
	};

	void update_portals(float dt);

	Portal& emplace_portal(entt::entity entity, const Portal& portal = {});
	bool remove_portal(entt::entity entity);

	bool activate_portal(entt::entity entity);
}