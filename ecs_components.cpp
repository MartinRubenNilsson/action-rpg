#include "ecs_components.h"

namespace ecs
{
	extern entt::registry _registry;

	void set_life_span(entt::entity entity, float life_span)
	{
		if (!_registry.valid(entity)) return;
		_registry.emplace_or_replace<LifeSpan>(entity, life_span);
	}
}
