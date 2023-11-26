#include "ecs_tiled.h"
#include "tiled.h"

namespace ecs
{
	extern entt::registry _registry;

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
		tiled::set(props, name, value);
	}

	template <typename T>
	bool _get_property(entt::entity entity, const std::string& name, T& value)
	{
		if (auto props = _registry.try_get<std::vector<tiled::Property>>(entity))
			return tiled::get(*props, name, value);
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
}
