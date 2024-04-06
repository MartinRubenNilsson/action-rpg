#include "stdafx.h"
#include "ecs_common.h"
#include "ecs_pickups.h"
#include "ecs_physics.h"
#include "ecs_tile.h"

namespace ecs
{
	extern entt::registry _registry;
	float _pickup_elapsed_time = 0.f;

	void update_pickups(float dt)
	{
		_pickup_elapsed_time += dt;

		// Blinking effect
		for (auto [entity, pickup, tile] : _registry.view<Pickup, Tile>().each()) {

			pickup.pickup_timer.update(dt);
			if (pickup.pickup_timer.finished()) {
				destroy_at_end_of_frame(entity);
				continue;
			}

			// Start blinking at >50% progress
			if (pickup.pickup_timer.get_progress() < 0.5f) continue;
			constexpr float BLINK_SPEED = 10.f;
			float blink_fraction = 0.75f + 0.25f * std::sin(_pickup_elapsed_time * BLINK_SPEED);
			tile.color.a = (sf::Uint8)(255 * blink_fraction);
		}
	}

	entt::entity create_pickup(PickupType type, const sf::Vector2f& position)
	{
		entt::entity entity = _registry.create();
		set_class(entity, "pickup");
		{
			Pickup& pickup = emplace_pickup(entity);
			pickup.type = type;
			pickup.pickup_timer.start();
		}
		{
			b2BodyDef body_def{};
			body_def.type = b2_staticBody;
			body_def.position.Set(position.x, position.y);
			b2Body* body = emplace_body(entity, body_def);
			b2CircleShape shape{};
			shape.m_radius = 4.f;
			b2FixtureDef fixture_def{};
			fixture_def.shape = &shape;
			fixture_def.isSensor = true;
			body->CreateFixture(&fixture_def);
		}
		{
			Tile& tile = emplace_tile(entity);
			switch (type) {
			case PickupType::Arrow:
				tile.set_tile("arrow", "items1");
				break;
			case PickupType::Rupee:
				tile.set_tile("rupee", "items1");
				break;
			case PickupType::Bomb:
				tile.set_tile("bomb", "items1");
				break;
			case PickupType::Heart:
				tile.set_tile("heart", "items1");
				break;
			}
			tile.position = position;
			tile.pivot = sf::Vector2f(8.f, 8.f);
			tile.sorting_pivot = sf::Vector2f(8.f, 16.f);
		}
		return entity;
	}

	Pickup& emplace_pickup(entt::entity entity) {
		return _registry.emplace_or_replace<Pickup>(entity);
	}

	bool remove_pickup(entt::entity entity) {
		return _registry.remove<Pickup>(entity);
	}
}

