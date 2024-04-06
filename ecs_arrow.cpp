#include "stdafx.h"
#include "ecs_arrow.h"
#include "ecs_common.h"
#include "ecs_tile.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "audio.h"

namespace ecs
{
	extern entt::registry _registry;

	entt::entity create_arrow(const sf::Vector2f& position, const sf::Vector2f& velocity)
	{
		entt::entity entity = _registry.create();
		set_class(entity, "arrow");

		// ARROW
		{
			Arrow& arrow = _registry.emplace<Arrow>(entity);
			arrow.damage = 1;
			arrow.lifetime = 0.f; // unused right now
		}

		// PHYSICS
		{
			b2BodyDef body_def{};
			body_def.type = b2_dynamicBody;
			body_def.position.Set(position.x, position.y);
			b2Body* body = emplace_body(entity, body_def);

			b2CircleShape shape{};
			shape.m_radius = 6.f;
			b2FixtureDef fixture_def{};
			fixture_def.shape = &shape;
			fixture_def.density = 1.f;
			fixture_def.filter = get_filter_for_class("arrow");
			body->CreateFixture(&fixture_def);

			body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
		}

		// GRAPHICS
		{
			Tile& tile = emplace_tile(entity);
			tile.set_tile("arrow", "items1");
			tile.pivot = sf::Vector2f(6.f, 6.f);
		}

		// AUDIO
		audio::play("event:/snd_fire_arrow");

		return entity;
	}
}