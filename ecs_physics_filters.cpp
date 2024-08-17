#include "stdafx.h"
#include "ecs_physics_filters.h"
#include "ecs_tags.h"

namespace ecs
{
	//TODO: make class into enum, make this into array
	const std::unordered_map<Tag, b2Filter> _TAG_TO_FILTER = {
		{ Tag::Player, make_filter(CC_Player, CM_Player) },
		{ Tag::Slime, make_filter(CC_Enemy) },
		{ Tag::Arrow, make_filter(CC_PlayerAttack) },
	};

	b2Filter make_filter(uint32_t category, uint32_t mask, int32_t group)
	{
		b2Filter filter{};
		filter.categoryBits = category;
		filter.maskBits = mask;
		filter.groupIndex = group;
		return filter;
	}

	b2Filter get_filter_for_tag(Tag tag)
	{
		auto it = _TAG_TO_FILTER.find(tag);
		if (it == _TAG_TO_FILTER.end()) return make_filter();
		return it->second;
	}
}
