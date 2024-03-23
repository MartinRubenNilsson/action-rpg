#pragma once

namespace ecs
{
	enum class PickupType
	{
		Arrow,
		Rupee,
		Bomb,
		Heart,
		Count,
	};

	struct Pickup
	{
		PickupType type = PickupType::Arrow;
	};

	void update_pickups(float dt);

	entt::entity create_pickup(PickupType type, const sf::Vector2f& position);

	Pickup& emplace_pickup(entt::entity entity);
	bool remove_pickup(entt::entity entity);
}
