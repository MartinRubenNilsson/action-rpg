#include "ecs_common.h"

namespace ecs
{
	extern entt::registry _registry;
	std::unordered_set<entt::entity> _entities_to_destroy;

	entt::entity find_entity_by_name(const std::string& name)
	{
		if (name.empty()) return entt::null;
		for (auto [entity, name_component] : _registry.view<Name>().each())
		{
			if (name_component.value == name)
				return entity;
		}
		return entt::null;
	}

	entt::entity find_entity_by_type(const std::string& type)
	{
		if (type.empty()) return entt::null;
		for (auto [entity, type_component] : _registry.view<Type>().each())
		{
			if (type_component.value == type)
				return entity;
		}
		return entt::null;
	}

	void set_name(entt::entity entity, const std::string& name)
	{
		if (!_registry.valid(entity)) return;
		_registry.emplace_or_replace<Name>(entity, name);
	}

	std::string get_name(entt::entity entity)
	{
		std::string name;
		if (auto ptr = _registry.try_get<Name>(entity))
			name = ptr->value;
		return name;
	}

	void set_type(entt::entity entity, const std::string& type)
	{
		if (!_registry.valid(entity)) return;
		_registry.emplace_or_replace<Type>(entity, type);
	}

	std::string get_type(entt::entity entity)
	{
		std::string type;
		if (auto ptr = _registry.try_get<Type>(entity))
			type = ptr->value;
		return type;
	}

	void set_properties(entt::entity entity, const Properties& properties)
	{
		if (!_registry.valid(entity)) return;
		_registry.emplace_or_replace<Properties>(entity, properties);
	}

	bool get_properties(entt::entity entity, Properties& properties)
	{
		if (auto ptr = _registry.try_get<Properties>(entity))
		{
			properties = *ptr;
			return true;
		}
		return false;
	}

	template <typename T>
	void _set_property(entt::entity entity, const std::string& name, const T& value)
	{
		if (!_registry.valid(entity)) return;
		auto& props = _registry.get_or_emplace<Properties>(entity).properties;
		for (auto& prop : props)
		{
			if (prop.name == name)
			{
				prop.value = value;
				return;
			}
		}
		props.emplace_back(name, value);
	}

	template <typename T>
	bool _get_property(entt::entity entity, const std::string& name, T& value)
	{
		if (auto ptr = _registry.try_get<Properties>(entity))
		{
			for (const auto& prop : ptr->properties)
			{
				if (prop.name == name && std::holds_alternative<T>(prop.value))
				{
					value = std::get<T>(prop.value);
					return true;
				}
			}
		}
		return false;
	}

	void set_bool(entt::entity entity, const std::string& name, bool value) {
		_set_property(entity, name, value);
	}

	bool get_bool(entt::entity entity, const std::string& name, bool& value) {
		return _get_property(entity, name, value);
	}

	void set_float(entt::entity entity, const std::string& name, float value) {
		_set_property(entity, name, value);
	}

	bool get_float(entt::entity entity, const std::string& name, float& value) {
		return _get_property(entity, name, value);
	}

	void set_int(entt::entity entity, const std::string& name, int value) {
		_set_property(entity, name, value);
	}

	bool get_int(entt::entity entity, const std::string& name, int& value) {
		return _get_property(entity, name, value);
	}

	void set_string(entt::entity entity, const std::string& name, const std::string& value) {
		_set_property(entity, name, value);
	}

	bool get_string(entt::entity entity, const std::string& name, std::string& value) {
		return _get_property(entity, name, value);
	}

	void set_entity(entt::entity entity, const std::string& name, entt::entity value) {
		_set_property(entity, name, value);
	}

	bool get_entity(entt::entity entity, const std::string& name, entt::entity& value) {
		return _get_property(entity, name, value);
	}

	void set_life_span(entt::entity entity, float life_span)
	{
		if (!_registry.valid(entity)) return;
		_registry.emplace_or_replace<LifeSpan>(entity, life_span);
	}

	void mark_for_destruction(entt::entity entity)
	{
		if (!_registry.valid(entity)) return;
		_entities_to_destroy.insert(entity);
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
