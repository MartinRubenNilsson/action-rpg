#include "ecs_tiled.h"
#include "tiled.h"

namespace ecs
{
	extern entt::registry _registry;

	void emplace_object(entt::entity entity, const tiled::Object* object) {
		_registry.emplace_or_replace<const tiled::Object*>(entity, object);
	}

	entt::entity find_entity_by_name(const std::string& name)
	{
		if (name.empty()) return entt::null;
		for (auto [entity, object] : _registry.view<const tiled::Object*>().each()) {
			if (object->name == name)
				return entity;
		}
		return entt::null;
	}

	entt::entity find_entity_by_class(const std::string& class_)
	{
		if (class_.empty()) return entt::null;
		for (auto [entity, object] : _registry.view<const tiled::Object*>().each()) {
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

	bool get_bool(entt::entity entity, const std::string& name, bool& value)
	{
		if (auto object = _registry.try_get<const tiled::Object*>(entity))
			return tiled::get((*object)->properties, name, value);
		return false;
	}

	bool get_float(entt::entity entity, const std::string& name, float& value)
	{
		if (auto object = _registry.try_get<const tiled::Object*>(entity))
			return tiled::get((*object)->properties, name, value);
		return false;
	}

	bool get_int(entt::entity entity, const std::string& name, int& value)
	{
		if (auto object = _registry.try_get<const tiled::Object*>(entity))
			return tiled::get((*object)->properties, name, value);
		return false;
	}

	bool get_string(entt::entity entity, const std::string& name, std::string& value)
	{
		if (auto object = _registry.try_get<const tiled::Object*>(entity))
			return tiled::get((*object)->properties, name, value);
		return false;
	}

	bool get_entity(entt::entity entity, const std::string& name, entt::entity& value)
	{
		if (auto object = _registry.try_get<const tiled::Object*>(entity))
			return tiled::get((*object)->properties, name, value);
		return false;
	}
}
