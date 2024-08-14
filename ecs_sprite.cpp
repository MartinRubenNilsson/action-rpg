#include "stdafx.h"
#include "ecs_sprite.h"

namespace ecs
{
	extern entt::registry _registry;

	void update_sprite_body_attachments()
	{
		for (auto [entity, sprite, body, attachment]  :
			_registry.view<sprites::Sprite, b2BodyId, SpriteBodyAttachment>().each()) {
			sprite.pos = b2Body_GetPosition(body) + attachment.position;
		}
	}

	sprites::Sprite& emplace_sprite(entt::entity entity)
	{
		return _registry.emplace_or_replace<sprites::Sprite>(entity);
	}

	sprites::Sprite* get_sprite(entt::entity entity)
	{
		return _registry.try_get<sprites::Sprite>(entity);
	}

	SpriteBodyAttachment& emplace_sprite_body_attachment(entt::entity entity, const Vector2f& position)
	{
		return _registry.emplace_or_replace<SpriteBodyAttachment>(entity, position);
	}

	SpriteBodyAttachment* get_sprite_body_attachment(entt::entity entity)
	{
		return _registry.try_get<SpriteBodyAttachment>(entity);
	}
}