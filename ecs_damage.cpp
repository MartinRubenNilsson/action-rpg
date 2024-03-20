#include "stdafx.h"
#include "ecs_damage.h"
#include "ecs_common.h"

namespace ecs
{
	extern entt::registry _registry;

	bool apply_damage(entt::entity entity, const Damage& damage)
	{
		if (!_registry.valid(entity)) return false;
		std::string class_ = get_class(entity);
		if (class_ == "player") {
			//return apply_damage_to_player(entity, damage);
			return false; //TODO
		} else if (class_ == "grass") {
			return apply_damage_to_grass(entity, damage);
		}
		return false;
	}

	bool apply_damage_to_grass(entt::entity entity, const Damage& damage)
	{
		//TODO
		return false;
	}
}