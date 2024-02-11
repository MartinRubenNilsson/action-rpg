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
		int amount = 0;
	};

	entt::entity create_arrow_pickup(const sf::Vector2f& position);
	entt::entity create_rupee_pickup(const sf::Vector2f& position, int amount);

	void emplace_pickup(entt::entity entity, const Pickup& pickup);
	void remove_pickup(entt::entity entity);

	void update_pickups(float dt);
}
