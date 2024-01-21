#include "ecs_tiled.h"
#include "tiled.h"

namespace ecs
{
	struct Name { std::string value; }; // for internal use only!
	struct Class { std::string value; }; // for internal use only!

	extern entt::registry _registry;

	void emplace_object(entt::entity entity, const tiled::Object* object) {
		_registry.emplace_or_replace<const tiled::Object*>(entity, object);
	}

	entt::entity find_entity_by_name(const std::string& name)
	{
		if (name.empty()) return entt::null;
		for (auto [entity, ecs_name] : _registry.view<const Name>().each())
			if (ecs_name.value == name) return entity;
		return entt::null;
	}

	entt::entity find_entity_by_class(const std::string& class_)
	{
		if (class_.empty()) return entt::null;
		for (auto [entity, ecs_class] : _registry.view<const Class>().each())
			if (ecs_class.value == class_) return entity;
		return entt::null;
	}

	void set_name(entt::entity entity, const std::string& name) {
		_registry.emplace_or_replace<Name>(entity, name);
	}

	void set_class(entt::entity entity, const std::string& class_) {
		_registry.emplace_or_replace<Class>(entity, class_);
	}

	std::string get_name(entt::entity entity) {
		return _registry.get_or_emplace<Name>(entity).value;
	}

	std::string get_class(entt::entity entity) {
		return _registry.get_or_emplace<Class>(entity).value;
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
