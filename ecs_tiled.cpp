#include "ecs_tiled.h"
#include "tiled.h"
#include "properties.h"

namespace ecs
{
	extern entt::registry _registry;

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

	void set_properties(entt::entity entity, const Properties& properties)
	{
		_registry.emplace_or_replace<Properties>(entity, properties);
	}

	bool get_properties(entt::entity entity, Properties& properties)
	{
		if (!_registry.all_of<Properties>(entity)) return false;
		properties = _registry.get<Properties>(entity);
		return true;
	}

	void set_bool(entt::entity entity, const std::string& name, bool value) {
		_registry.get_or_emplace<Properties>(entity).set_bool(name, value);
	}

	void set_float(entt::entity entity, const std::string& name, float value) {
		_registry.get_or_emplace<Properties>(entity).set_float(name, value);
	}

	void set_int(entt::entity entity, const std::string& name, int value) {
		_registry.get_or_emplace<Properties>(entity).set_int(name, value);
	}

	void set_string(entt::entity entity, const std::string& name, const std::string& value) {
		_registry.get_or_emplace<Properties>(entity).set_string(name, value);
	}

	void set_entity(entt::entity entity, const std::string& name, entt::entity value) {
		_registry.get_or_emplace<Properties>(entity).set_entity(name, value);
	}

	bool get_bool(entt::entity entity, const std::string& name, bool& value)
	{
		auto props = _registry.try_get<const Properties>(entity);
		return props && props->get_bool(name, value);
	}

	bool get_float(entt::entity entity, const std::string& name, float& value)
	{
		auto props = _registry.try_get<const Properties>(entity);
		return props && props->get_float(name, value);
	}

	bool get_int(entt::entity entity, const std::string& name, int& value)
	{
		auto props = _registry.try_get<const Properties>(entity);
		return props && props->get_int(name, value);
	}

	bool get_string(entt::entity entity, const std::string& name, std::string& value)
	{
		auto props = _registry.try_get<const Properties>(entity);
		return props && props->get_string(name, value);
	}

	bool get_entity(entt::entity entity, const std::string& name, entt::entity& value)
	{
		auto props = _registry.try_get<const Properties>(entity);
		return props && props->get_entity(name, value);
	}
}
