#include "stdafx.h"
#include "ecs_pickups.h"
#include "ecs_common.h"
#include "ecs_sprite.h"
#include "ecs_physics.h"
#include "ecs_animation.h"
#include "tile_ids.h"

namespace map
{
	unsigned int get_object_layer_index();
}

namespace ecs
{
	extern entt::registry _registry;
	float _pickup_elapsed_time = 0.f;

	void update_pickups(float dt)
	{
		_pickup_elapsed_time += dt;

		// Blinking effect
		for (auto [entity, pickup, sprite] : _registry.view<Pickup, sprites::Sprite>().each()) {

			pickup.pickup_timer.update(dt);
			if (pickup.pickup_timer.finished()) {
				destroy_at_end_of_frame(entity);
				continue;
			}

			// Start blinking at >50% progress
			if (pickup.pickup_timer.get_progress() < 0.5f) continue;

			constexpr float BLINK_SPEED = 10.f;
			float blink_fraction = 0.75f + 0.25f * std::sin(_pickup_elapsed_time * BLINK_SPEED);
			sprite.color.a = (unsigned char)(255 * blink_fraction);
		}
	}

	entt::entity create_pickup(PickupType type, const Vector2f& position)
	{
		entt::entity entity = _registry.create();
		set_class(entity, "pickup");
		{
			Pickup& pickup = emplace_pickup(entity);
			pickup.type = type;
			pickup.pickup_timer.start();
		}
		{
			b2BodyDef body_def = b2DefaultBodyDef();
			body_def.type = b2_staticBody;
			body_def.position = position;
			b2BodyId body = emplace_body(entity, body_def);

#if 0
			b2CircleShape shape{};
			shape.m_radius = 4.f;

			b2FixtureDef fixture_def{};
			fixture_def.shape = &shape;
			fixture_def.isSensor = true;
			body->CreateFixture(&fixture_def);
#endif
		}
		{
			sprites::Sprite& sprite = emplace_sprite(entity);
			sprite.position = position - Vector2f(8.f, 8.f);
			sprite.size = { 16.f, 16.f };
			sprite.sorting_point = { 8.f, 8.f };
			sprite.sorting_layer = (uint8_t)map::get_object_layer_index();
		}
		{
			Animation& animation = emplace_animation(entity);
			animation.tileset = get_tileset("items1");
			switch (type) {
			case PickupType::Arrow:
				animation.tile_id = TILE_ID_ITEM_SPEAR; // placeholder
				break;
			case PickupType::Rupee:
				animation.tile_id = TILE_ID_ITEM_RUPEE;
				break;
			case PickupType::Bomb:
				animation.tile_id = TILE_ID_ITEM_POTION; // placeholder
				break;
			case PickupType::Heart:
				animation.tile_id = TILE_ID_ITEM_BERRIES; // placeholder
				break;
			}
		}
		return entity;
	}

	Pickup& emplace_pickup(entt::entity entity)
	{
		return _registry.emplace_or_replace<Pickup>(entity);
	}

	Pickup* get_pickup(entt::entity entity)
	{
		return _registry.try_get<Pickup>(entity);
	}

	bool remove_pickup(entt::entity entity)
	{
		return _registry.remove<Pickup>(entity);
	}
}

