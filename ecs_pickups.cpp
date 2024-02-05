#include "stdafx.h"
#include "ecs_tiled.h"
#include "ecs_pickups.h"
#include "ecs_physics.h"
#include "ecs_tile.h"

namespace ecs
{
	extern entt::registry _registry;
	float _pickup_elapsed_time = 0.f;

	entt::entity create_arrow_pickup(const sf::Vector2f& position)
	{
		entt::entity entity = _registry.create();

		set_class(entity, "pickup");

		Pickup pickup{};
		pickup.type = PickupType::Arrow;
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

		if (Tile* tile = emplace_tile(entity, "items1", "arrow")) {
			tile->position = position;
			tile->pivot = sf::Vector2f(16.f / 2.f, 16.f / 2.f);
		}

		return entity;
	}

	void emplace_pickup(entt::entity entity, const Pickup& pickup) {
		_registry.emplace_or_replace<Pickup>(entity, pickup);
	}

	void remove_pickup(entt::entity entity) {
		_registry.remove<Pickup>(entity);
	}

	void update_pickups(float dt)
	{
		_pickup_elapsed_time += dt;

		// Blinking effect
		for (auto [entity, pickup, tile] : _registry.view<Pickup, Tile>().each()) {
			constexpr float BLINK_SPEED = 10.f;
			float blink_fraction = 0.75f + 0.25f * std::sin(_pickup_elapsed_time * BLINK_SPEED);
			tile.color.a = (sf::Uint8)(255 * blink_fraction);
		}
	}
}

