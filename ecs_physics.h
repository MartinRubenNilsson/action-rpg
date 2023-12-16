#pragma once

namespace ecs
{
	void initialize_physics();

	void emplace_body(entt::entity entity, b2Body* body);

	std::vector<entt::entity> query_aabb(const sf::Vector2f& min, const sf::Vector2f& max);
}

