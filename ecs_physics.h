#pragma once

namespace ecs
{
	void initialize_physics();

	// TODO: raycast

	std::vector<entt::entity> query_aabb(const sf::Vector2f& min, const sf::Vector2f& max);
}

