#include "ecs_common.h"

namespace ecs
{
	extern entt::registry _registry;
	std::unordered_set<entt::entity> _entities_to_destroy;

	void mark_for_destruction(entt::entity entity) {
		_entities_to_destroy.insert(entity);
	}

	std::string get_name(entt::entity entity)
	{
		std::string name;
		if (auto object = _registry.try_get<const tmx::Object*>(entity))
			name = (*object)->getName();
		return name;
	}

	std::string get_type(entt::entity entity)
	{
		std::string type;
		if (auto object = _registry.try_get<const tmx::Object*>(entity))
			type = (*object)->getType();
		return type;
	}

	void set_life_span(entt::entity entity, float life_span)
	{
		if (!_registry.valid(entity)) return;
		_registry.emplace_or_replace<LifeSpan>(entity, life_span);
	}

	void _update_life_spans(float dt)
	{
		for (auto [entity, life_span] : _registry.view<LifeSpan>().each())
		{
			life_span.value -= dt;
			if (life_span.value <= 0)
				mark_for_destruction(entity);
		}
	}

	void _destroy_marked_entities()
	{
		for (entt::entity entity : _entities_to_destroy)
			if (_registry.valid(entity))
				_registry.destroy(entity);
		_entities_to_destroy.clear();
	}

	void update_common(float dt)
	{
		_update_life_spans(dt);
		_destroy_marked_entities();
	}
}
