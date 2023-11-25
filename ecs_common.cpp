#include "ecs_common.h"
#include "tiled.h"

namespace ecs
{
	extern entt::registry _registry;
	std::unordered_set<entt::entity> _entities_to_destroy;

	entt::entity find_entity_by_name(const std::string& name)
	{
		if (name.empty()) return entt::null;
		for (auto [entity, object] : _registry.view<const tiled::Object*>().each())
		{
			if (object->name == name)
				return entity;
		}
		return entt::null;
	}

	entt::entity find_entity_by_class(const std::string& class_)
	{
		if (class_.empty()) return entt::null;
		for (auto [entity, object] : _registry.view<const tiled::Object*>().each())
		{
			if (object->class_ == class_)
				return entity;
		}
		return entt::null;
	}

	std::string get_name(entt::entity entity)
	{
		std::string name;
		if (auto object = _registry.try_get<const tiled::Object*>(entity))
			name = (*object)->name;
		return name;
	}

	std::string get_class(entt::entity entity)
	{
		std::string type;
		if (auto object = _registry.try_get<const tiled::Object*>(entity))
			type = (*object)->class_;
		return type;
	}

	template <typename T>
	void _set_property(entt::entity entity, const std::string& name, const T& value)
	{
		if (!_registry.valid(entity)) return;
		auto& props = _registry.get_or_emplace<std::vector<tiled::Property>>(entity);
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
		if (auto props = _registry.try_get<std::vector<tiled::Property>>(entity))
		{
			for (const auto& prop : *props)
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

	void mark_for_destruction(entt::entity entity)
	{
		if (!_registry.valid(entity)) return;
		_entities_to_destroy.insert(entity);
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
		_destroy_marked_entities();
	}
}
