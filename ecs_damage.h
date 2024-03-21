#pragma once

namespace ecs
{
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
		//sf::Vector2f direction; //Could be useful for knockback
	};

	void apply_damage_in_box(const Damage& damage, const sf::Vector2f& box_min, const sf::Vector2f& box_max, uint16_t mask_bits = 0xFFFF);

	//TODO: apply_radial_damage, apply_box_damage, apply_cone_damage, etc.

	// Delegates to the appropriate function based on the entity's class (e.g. player, enemy, etc.)
	// Returns true if the entity was damaged, false otherwise.
	bool apply_damage(entt::entity entity, const Damage& damage);

	// CLASS-SPECIFIC DAMAGE FUNCTIONS

	bool apply_damage_to_player(entt::entity entity, const Damage& damage);
	bool apply_damage_to_slime(entt::entity entity, const Damage& damage);
	bool apply_damage_to_bomb(entt::entity entity, const Damage& damage);
	bool apply_damage_to_grass(entt::entity entity, const Damage& damage);
}

