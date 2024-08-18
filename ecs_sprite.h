#pragma once
#include "sprites.h"

namespace ecs
{
	// Causes the sprites::Sprite's position to follow the b2BodyId's position.
	struct SpriteFollowBody
	{
		Vector2f offset; // the sprite's position relative to the body's position
	};

	struct SpriteShake
	{
		// The duration decreases by dt each frame, while the magnitude decreases
		// in such a way that the two values always satisfy the following power law:
		// 
		//     magnitude = initial_magnitude * pow(duration / initial_duration, exponent).
		//
		// The magnitude remains constant when exponent = 0, decreases linearly when exponent = 1,
		// decreases faster at the end when 0 < exponent < 1, and decreases faster initially
		// when exponent > 1. A natural choice for a shake that fades out is exponent = 2.

		float duration = 0.f;
		float magnitude = 0.f;
		float exponent = 0.f;
		Vector2f _original_position; // for internal use!
	};

	void update_sprites_following_bodies();
	void update_sprite_shakes(float dt);
	void shake_sprites_before_drawing();
	void unshake_sprites_after_drawing();

	sprites::Sprite& emplace_sprite(entt::entity entity);
	sprites::Sprite* get_sprite(entt::entity entity);

	SpriteFollowBody& emplace_sprite_follow_body(entt::entity entity, const Vector2f& offset = { 0.f, 0.f });
	SpriteFollowBody* get_sprite_follow_body(entt::entity entity);

	SpriteShake& emplace_sprite_shake(entt::entity entity);
}