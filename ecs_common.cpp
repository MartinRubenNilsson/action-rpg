#include "ecs_common.h"

namespace ecs
{
	extern entt::registry _registry;

	void set_name(entt::entity entity, const std::string& name)
	{
		if (!_registry.valid(entity)) return;
		_registry.emplace_or_replace<Name>(entity, name);
	}

	bool has_name(entt::entity entity, const std::string& name)
	{
		if (!_registry.all_of<Name>(entity)) return false;
		return _registry.get<Name>(entity).value == name;
	}

	void set_type(entt::entity entity, const std::string& type)
	{
		if (!_registry.valid(entity)) return;
		_registry.emplace_or_replace<Type>(entity, type);
	}

	bool has_type(entt::entity entity, const std::string& type)
	{
		if (!_registry.all_of<Type>(entity)) return false;
		return _registry.get<Type>(entity).value == type;
	}

	void set_life_span(entt::entity entity, float life_span)
	{
		if (!_registry.valid(entity)) return;
		_registry.emplace_or_replace<LifeSpan>(entity, life_span);
	}
}
