#pragma once

namespace ecs
{
	enum class PickupType
	{
		Arrow,
	};

	struct Pickup
	{
		PickupType type;
		// TODO: parameter types like "amount", etc.
	};

	entt::entity create_arrow_pickup(const sf::Vector2f& position);

	void emplace_pickup(entt::entity entity, const Pickup& pickup);
	void remove_pickup(entt::entity entity);

	void update_pickups(float dt);
}
