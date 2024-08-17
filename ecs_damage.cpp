#include "stdafx.h"
#include "ecs_damage.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_pickups.h"
#include "random.h"
#include "audio.h"

namespace ecs
{
	std::unordered_set<entt::entity> _entities_that_took_damage;

	bool apply_damage_in_box(const Damage& damage, const Vector2f& box_min, const Vector2f& box_max, uint32_t mask_bits)
	{
		//debug::draw_box(box_min, box_max, Color::Red, 0.2f);
		for (const OverlapHit& hit : overlap_box(box_min, box_max, mask_bits)) {
			if (hit.entity == damage.source) continue;
			if (_entities_that_took_damage.contains(hit.entity)) continue;
			if (!apply_damage(hit.entity, damage)) continue;
			_entities_that_took_damage.insert(hit.entity);
		}
		bool any_damaged = !_entities_that_took_damage.empty();
		_entities_that_took_damage.clear();
		return any_damaged;
	}

	bool apply_damage_in_circle(const Damage& damage, const Vector2f& center, float radius, uint32_t mask_bits)
	{
		//debug::draw_circle(center, radius, Color::Red, 0.2f);
		for (const OverlapHit& hit : overlap_circle(center, radius, mask_bits)) {
			if (hit.entity == damage.source) continue;
			if (_entities_that_took_damage.contains(hit.entity)) continue;
			if (!apply_damage(hit.entity, damage)) continue;
			_entities_that_took_damage.insert(hit.entity);
		}
		bool any_damaged = !_entities_that_took_damage.empty();
		_entities_that_took_damage.clear();
		return any_damaged;
	}

	bool apply_damage(entt::entity entity, const Damage& damage)
	{
		switch (get_tag(entity)) {
		case Tag::Player:
			return apply_damage_to_player(entity, damage);
		case Tag::Slime:
			return apply_damage_to_slime(entity, damage);
		case Tag::Bomb:
			return apply_damage_to_bomb(entity, damage);
		case Tag::Grass:
			return apply_damage_to_grass(entity, damage);
		}
		return false;
	}

	bool apply_damage_to_slime(entt::entity entity, const Damage& damage)
	{
		//TODO: more stuff here

		audio::create_event({ .path = "event:/snd_slime_dying" });

		// TODO use snd_slime_hurt when slime is damaged and snd_slime_dying when slime is dead
		// audio::play("event:/snd_slime_hurt");

		destroy_at_end_of_frame(entity);
		return true;
	}

	bool apply_damage_to_grass(entt::entity entity, const Damage& damage)
	{
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