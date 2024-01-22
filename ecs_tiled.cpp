#include "ecs_tiled.h"
#include "tiled.h"

namespace ecs
{
	struct Name { std::string value; }; // for internal use only!
	struct Class { std::string value; }; // for internal use only!

	extern entt::registry _registry;

	void emplace_name_class_and_properties(entt::entity entity, const tiled::Object& object)
	{
		if (!object.name.empty())
			_registry.emplace_or_replace<Name>(entity, object.name);
		if (!object.class_.empty())
			_registry.emplace_or_replace<Class>(entity, object.class_);
		if (!object.properties.empty())
			_registry.emplace_or_replace<std::vector<tiled::Property>>(entity, object.properties);
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

	std::string get_name(entt::entity entity)
	{
		if (auto name = _registry.try_get<const Name>(entity))
			return name->value;
		return "";
	}

	std::string get_class(entt::entity entity)
	{
		if (auto class_ = _registry.try_get<const Class>(entity))
			return class_->value;
		return "";
	}

	void set_bool(entt::entity entity, const std::string& name, bool value) {
		tiled::set(_registry.emplace_or_replace<std::vector<tiled::Property>>(entity), name, value);
	}

	void set_float(entt::entity entity, const std::string& name, float value) {
		tiled::set(_registry.emplace_or_replace<std::vector<tiled::Property>>(entity), name, value);
	}

	void set_int(entt::entity entity, const std::string& name, int value) {
		tiled::set(_registry.emplace_or_replace<std::vector<tiled::Property>>(entity), name, value);
	}

	void set_string(entt::entity entity, const std::string& name, const std::string& value) {
		tiled::set(_registry.emplace_or_replace<std::vector<tiled::Property>>(entity), name, value);
	}

	void set_entity(entt::entity entity, const std::string& name, entt::entity value) {
		tiled::set(_registry.emplace_or_replace<std::vector<tiled::Property>>(entity), name, value);
	}

	bool get_bool(entt::entity entity, const std::string& name, bool& value)
	{
		if (auto props = _registry.try_get<const std::vector<tiled::Property>>(entity))
			return tiled::get(*props, name, value);
		return false;
	}

	bool get_float(entt::entity entity, const std::string& name, float& value)
	{
		if (auto props = _registry.try_get<const std::vector<tiled::Property>>(entity))
			return tiled::get(*props, name, value);
		return false;
	}

	bool get_int(entt::entity entity, const std::string& name, int& value)
	{
		if (auto props = _registry.try_get<const std::vector<tiled::Property>>(entity))
			return tiled::get(*props, name, value);
		return false;
	}

	bool get_string(entt::entity entity, const std::string& name, std::string& value)
	{
		if (auto props = _registry.try_get<const std::vector<tiled::Property>>(entity))
			return tiled::get(*props, name, value);
		return false;
	}

	bool get_entity(entt::entity entity, const std::string& name, entt::entity& value)
	{
		if (auto props = _registry.try_get<const std::vector<tiled::Property>>(entity))
			return tiled::get(*props, name, value);
		return false;
	}
}
