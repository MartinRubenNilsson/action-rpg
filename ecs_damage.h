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

	// Delegates to the appropriate function based on the entity's class (e.g. player, enemy, etc.)
	// Returns true if the entity was damaged, false otherwise.
	bool apply_damage(entt::entity entity, const Damage& damage);

	// CLASS-SPECIFIC DAMAGE FUNCTIONS

	bool apply_damage_to_player(entt::entity entity, const Damage& damage);
	bool apply_damage_to_slime(entt::entity entity, const Damage& damage);
	bool apply_damage_to_grass(entt::entity entity, const Damage& damage);
}

