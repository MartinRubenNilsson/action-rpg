#include "ecs_common.h"

namespace ecs
{
	extern entt::registry _registry;

	bool has_name(entt::entity entity, const std::string& name)
	{
		if (!_registry.all_of<const tmx::Object*>(entity)) return false;
		return _registry.get<const tmx::Object*>(entity)->getName() == name;
	}

	bool has_type(entt::entity entity, const std::string& type)
	{
		if (!_registry.all_of<const tmx::Object*>(entity)) return false;
		return _registry.get<const tmx::Object*>(entity)->getType() == type;
	}

	void set_life_span(entt::entity entity, float life_span)
	{
		if (!_registry.valid(entity)) return;
		_registry.emplace_or_replace<LifeSpan>(entity, life_span);
	}
}
