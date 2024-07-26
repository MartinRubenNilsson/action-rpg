#pragma once

namespace ecs
{
	//Not really used right now, but could be useful for future features.
	enum class DamageType
	{
		Default,
		Melee,
		Projectile,
		Explosion,
		//Magic,
		//Environment,
	};

	struct Damage
	{
		DamageType type = DamageType::Default;
		int amount = 0;
		// The entity that dealt the damage (e.g. the bomb that exploded).
		// As a safety measure, the source entity cannot damage itself.
		entt::entity source = entt::null; 
	};

	// AREA-BASED DAMAGE FUNCTIONS

	// Applies damage to all entities that intersect the given box. Returns true if any entity was damaged.
	bool apply_damage_in_box(const Damage& damage, const Vector2f& box_min, const Vector2f& box_max, uint16_t mask_bits = 0xFFFF);
	// Applies damage to all entities that intersect the given circle. Returns true if any entity was damaged.
	bool apply_damage_in_circle(const Damage& damage, const Vector2f& center, float radius, uint16_t mask_bits = 0xFFFF);

	// UNIVERSAL DAMAGE FUNCTION

	// Delegates to the appropriate function based on the entity's class (e.g. player, enemy, etc.)
	// Returns true if the entity was damaged, false if it was immune or invulnerable.
	bool apply_damage(entt::entity entity, const Damage& damage);

	// CLASS-SPECIFIC DAMAGE FUNCTIONS

	bool apply_damage_to_player(entt::entity entity, const Damage& damage);
	bool apply_damage_to_slime(entt::entity entity, const Damage& damage);
	bool apply_damage_to_bomb(entt::entity entity, const Damage& damage);
	bool apply_damage_to_grass(entt::entity entity, const Damage& damage);
}

