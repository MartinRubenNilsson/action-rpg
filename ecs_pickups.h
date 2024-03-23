#pragma once

namespace ecs
{
	enum class PickupType
	{
		Arrow,
		Rupee,
	};

	struct Pickup
	{
		PickupType type;
	};

	void update_pickups(float dt);

	entt::entity create_arrow_pickup(const sf::Vector2f& position);
	entt::entity create_rupee_pickup(const sf::Vector2f& position);

	void emplace_pickup(entt::entity entity, const Pickup& pickup);
	bool remove_pickup(entt::entity entity);
}
