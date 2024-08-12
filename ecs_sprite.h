#pragma once
#include "sprites.h"

namespace ecs
{
	// Makes the sprite follow the b2Body.
	struct SpriteBodyAttachment
	{
		Vector2f position; // position of the sprite relative to the body
	};

	void update_sprite_body_attachments();

	sprites::Sprite& emplace_sprite(entt::entity entity);
	sprites::Sprite* get_sprite(entt::entity entity);

	SpriteBodyAttachment& emplace_sprite_body_attachment(entt::entity entity, const Vector2f& position = { 0.f, 0.f });
	SpriteBodyAttachment* get_sprite_body_attachment(entt::entity entity);
}