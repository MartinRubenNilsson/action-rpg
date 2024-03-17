#include "stdafx.h"
#include "ecs_physics_filters.h"

namespace ecs
{
	const std::unordered_map<std::string, b2Filter> _CLASS_TO_FILTER = {
		{ "player", make_filter(CC_Player, CM_Player) },
		{ "slime", make_filter(CC_Enemy) },
		{ "arrow", make_filter(CC_PlayerAttack) },
	};

	b2Filter make_filter(uint16_t category, uint16_t mask, int16_t group)
	{
		b2Filter filter;
		filter.categoryBits = category;
		filter.maskBits = mask;
		filter.groupIndex = group;
		return filter;
	}

	b2Filter get_filter_for_class(const std::string& class_)
	{
		auto it = _CLASS_TO_FILTER.find(class_);
		if (it != _CLASS_TO_FILTER.end()) return it->second;
		return make_filter();
	}
}
