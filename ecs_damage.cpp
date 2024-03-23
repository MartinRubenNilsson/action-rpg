#include "stdafx.h"
#include "ecs_damage.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_pickups.h"
#include "physics_helpers.h"
#include "random.h"
#include "audio.h"
#include "debug_draw.h"

namespace ecs
{
	extern entt::registry _registry;

	void apply_damage_in_box(const Damage& damage, const sf::Vector2f& box_min, const sf::Vector2f& box_max, uint16_t mask_bits)
	{
		debug::draw_box(box_min, box_max, sf::Color::Red, 0.2f);
		for (const OverlapHit& hit : overlap_box(box_min, box_max, mask_bits)) {
			if (hit.entity == damage.source) continue;
			// TODO: don't apply damage to the same entity multiple times
			apply_damage(hit.entity, damage);
		}
	}

	void apply_damage_in_circle(const Damage& damage, const sf::Vector2f& center, float radius, uint16_t mask_bits)
	{
		debug::draw_circle(center, radius, sf::Color::Red, 0.2f);
		for (const OverlapHit& hit : overlap_circle(center, radius, mask_bits)) {
			if (hit.entity == damage.source) continue;
			// TODO: don't apply damage to the same entity multiple times
			apply_damage(hit.entity, damage);
		}
	}

	bool apply_damage(entt::entity entity, const Damage& damage)
	{
		if (!_registry.valid(entity)) return false;
		std::string class_ = get_class(entity);
		if (class_ == "player") {
			return apply_damage_to_player(entity, damage);
		} else if (class_ == "slime") {
			return apply_damage_to_slime(entity, damage);
		} else if (class_ == "bomb") {
			return apply_damage_to_bomb(entity, damage);
		} else if (class_ == "grass") {
			return apply_damage_to_grass(entity, damage);
		}
		return false;
	}

	bool apply_damage_to_slime(entt::entity entity, const Damage& damage)
	{
		//TODO: more stuff here
		audio::play("event:/snd_slime_dying");
		destroy_at_end_of_frame(entity);
		return true;
	}

	bool apply_damage_to_grass(entt::entity entity, const Damage& damage)
	{
		if (!has_body(entity)) return false;
		b2Body* body = get_body(entity);
		sf::Vector2f position = get_world_center(body);
		audio::play("event:/snd_cut_grass");
		if (random::chance(0.2f)) {
			int rand_int = random::range_i(0, (int)PickupType::Count - 1);
			PickupType pickup_type = (PickupType)rand_int;
			create_pickup(position + sf::Vector2f(2.f, 2.f), pickup_type);
		}
		destroy_at_end_of_frame(entity);

		return true;
	}
}