#include "stdafx.h"
#include "ecs_common.h"
#include "ecs_pickups.h"
#include "ecs_physics.h"
#include "ecs_tile.h"

namespace ecs
{
	extern entt::registry _registry;
	float _pickup_elapsed_time = 0.f;

	entt::entity create_pickup(const sf::Vector2f& position, PickupType pickup_type)
	{
		entt::entity entity = _registry.create();

		set_class(entity, "pickup");

		Pickup pickup{};
		pickup.type = pickup_type;
		_registry.emplace<Pickup>(entity, pickup);

		b2BodyDef body_def;
		body_def.type = b2_staticBody;
		body_def.position.Set(position.x, position.y);
		if (b2Body* body = emplace_body(entity, body_def)) {
			b2CircleShape shape;
			shape.m_radius = 4.f;
			b2FixtureDef fixture_def;
			fixture_def.shape = &shape;
			fixture_def.isSensor = true;
			body->CreateFixture(&fixture_def);
		}

		Tile& tile = emplace_tile(entity);

		switch (pickup_type) {
		case PickupType::Arrow:
			tile.set_sprite("arrow", "items1");
			break;
		case PickupType::Rupee:
			tile.set_sprite("rupee", "items1");
			break;
		case PickupType::Bomb:
			tile.set_sprite("bomb", "items1");
			break;
		case PickupType::Heart:
			tile.set_sprite("heart", "items1");
			break;
		}

		tile.position = position;
		tile.pivot = sf::Vector2f(8.f, 8.f);
		tile.sorting_pivot = sf::Vector2f(8.f, 16.f);

		return entity;
	}

	void emplace_pickup(entt::entity entity, const Pickup& pickup) {
		_registry.emplace_or_replace<Pickup>(entity, pickup);
	}

	bool remove_pickup(entt::entity entity) {
		return _registry.remove<Pickup>(entity);
	}
}

