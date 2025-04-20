#include "stdafx.h"
#include "ecs_grass.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_pickups.h"
#include "random.h"
#include "audio.h"

namespace ecs {
	extern entt::registry _registry;

	Grass& emplace_grass(entt::entity entity) {
		return _registry.emplace_or_replace<Grass>(entity);
	}

	GrassUniformBlock& emplace_grass_uniform_block(entt::entity entity) {
		return _registry.emplace_or_replace<GrassUniformBlock>(entity);
	}

	bool apply_damage_to_grass(entt::entity entity, const Damage& damage) {
		b2BodyId body = get_body(entity);
		if (B2_IS_NULL(body)) return false;
		const Vector2f position = b2Body_GetPosition(body);
		audio::create_event({ .path = "event:/snd_cut_grass" });
		if (random::chance(0.2f)) {
			PickupType pickup_type = (PickupType)random::range_i(0, (int)PickupType::Count - 1);
			create_pickup(pickup_type, position + Vector2f(8.f, 20.f));
		}
		destroy_at_end_of_frame(entity);
		return true;
	}
}
