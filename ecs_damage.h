#pragma once

namespace ecs {
	//Not really used right now, but could be useful for future features.
	enum class DamageType {
		Default,
		Melee,
		Projectile,
		Explosion,
		//Magic,
		//Environment,
	};

	struct Damage {
		DamageType type = DamageType::Default;
		int amount = 0;
		// The entity that dealt the damage (e.g. the bomb that exploded).
		// As a safety measure, the source entity cannot damage itself.
		entt::entity source = entt::null;
	};

	// Called when the entity is meant to take damage. Should return true if any damage was applied.
	using ApplyDamageCallback = bool(*)(entt::entity entity, const Damage& damage);

	void set_apply_damage_callback(entt::entity entity, ApplyDamageCallback callback);
	ApplyDamageCallback get_apply_damage_callback(entt::entity entity); // Returns nullptr if callback isn't set

	// Appplies damage to a single entity by calling the entity's ApplyDamageCallback.
	bool apply_damage(entt::entity entity, const Damage& damage);

	// Applies damage to all entities that intersect the given box. Returns true if any entity was damaged.
	bool apply_damage_in_box(const Damage& damage, const Vector2f& box_min, const Vector2f& box_max, uint32_t mask_bits = UINT32_MAX);
	// Applies damage to all entities that intersect the given circle. Returns true if any entity was damaged.
	bool apply_damage_in_circle(const Damage& damage, const Vector2f& center, float radius, uint32_t mask_bits = UINT32_MAX);
}
