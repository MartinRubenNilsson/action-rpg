#include "stdafx.h"
#include "ecs_arrow.h"
#include "ecs_common.h"
#include "ecs_sprite.h"
#include "ecs_animation.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "tile_ids.h"
#include "audio.h"

namespace map
{
	unsigned int get_object_layer_index();
}

namespace ecs
{
	extern entt::registry _registry;

	Arrow& emplace_arrow(entt::entity entity, const Arrow& arrow)
	{
		return _registry.emplace_or_replace<Arrow>(entity, arrow);
	}

	entt::entity create_arrow(const Vector2f& position, const Vector2f& velocity)
	{
		entt::entity entity = _registry.create();
		set_tag(entity, Tag::Arrow);
		{
			Arrow& arrow = emplace_arrow(entity);
			arrow.damage = 1;
			arrow.lifetime = 0.f; // unused right now
		}
		const Vector2f pivot = { 8.f, 8.f };
		{
			sprites::Sprite& sprite = emplace_sprite(entity);
			sprite.sorting_layer = map::get_object_layer_index();
			sprite.sorting_point = pivot;
			sprite.position = position - pivot;
			sprite.size = { 16.f, 16.f };
		}
		{
			Animation& animation = emplace_animation(entity);
			animation.tileset = get_tileset("items1");
			animation.tile_id = TILE_ID_ITEM_SPEAR; // placeholder
		}
		{
			b2BodyDef body_def = b2DefaultBodyDef();
			body_def.type = b2_dynamicBody;
			body_def.position = position;
			body_def.linearVelocity = velocity;
			b2BodyId body = emplace_body(entity, body_def);
			b2ShapeDef shape_def = b2DefaultShapeDef();
			shape_def.filter = get_physics_filter_for_tag(Tag::Arrow);
			b2Circle circle{};
			circle.radius = 6.f;
			b2CreateCircleShape(body, &shape_def, &circle);
		}
		emplace_sprite_follow_body(entity, -pivot);
		audio::create_event({ .path = "event:/snd_fire_arrow" });
		return entity;
	}
}