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
		PickupType type;
	};

	entt::entity create_pickup(const sf::Vector2f& position, PickupType pickup);

	void emplace_pickup(entt::entity entity, const Pickup& pickup);
	void remove_pickup(entt::entity entity);

	void update_pickups(float dt);
}
