#include "stdafx.h"
#include "ecs_physics_filters.h"

namespace ecs
{
	//TODO: make class into enum, make this into array
	const std::unordered_map<std::string, b2Filter> _CLASS_TO_FILTER = {
		{ "player", make_filter(CC_Player, CM_Player) },
		{ "slime", make_filter(CC_Enemy) },
		{ "arrow", make_filter(CC_PlayerAttack) },
	};

	b2Filter make_filter(uint32_t category, uint32_t mask, int32_t group)
	{
		b2Filter filter{};
		filter.categoryBits = category;
		filter.maskBits = mask;
		filter.groupIndex = group;
		return filter;
	}

	b2Filter get_filter_for_class(const std::string& class_)
	{
		auto it = _CLASS_TO_FILTER.find(class_);
		if (it == _CLASS_TO_FILTER.end()) return make_filter();
		return it->second;
	}
}
