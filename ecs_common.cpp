#include "stdafx.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_player.h"
#include "tiled.h"
#include "properties.h"

namespace ecs
{
	struct Name { std::string value; };

	extern entt::registry _registry;
	std::unordered_set<entt::entity> _entities_to_destroy_at_end_of_frame;

	void update_lifetimes(float dt)
	{
		for (auto [entity, lifetime] : _registry.view<Lifetime>().each()) {
			lifetime.time -= dt;
			if (lifetime.time <= 0.f) {
				destroy_at_end_of_frame(entity);
			}
		}
	}

	void destroy_entities_to_be_destroyed_at_end_of_frame()
	{
		for (entt::entity entity : _entities_to_destroy_at_end_of_frame) {
			if (_registry.valid(entity)) {
				_registry.destroy(entity);
			}
		}
		_entities_to_destroy_at_end_of_frame.clear();
	}

	void clear()
	{
		_registry.clear();
		_entities_to_destroy_at_end_of_frame.clear();
	}

	entt::entity create()
	{
		return _registry.create();
	}

	entt::entity create(entt::entity hint)
	{
		return _registry.create(hint);
	}

	entt::entity deep_copy(entt::entity entity)
	{
		entt::entity copied_entity = _registry.create();
		for (auto [name, storage] : _registry.storage()) {
			if (!storage.contains(entity)) continue;
			if (storage.type() == entt::type_id<b2BodyId>()) {
				deep_copy_and_emplace_body(copied_entity, *(b2BodyId*)storage.value(entity));
			} else if (storage.type() == entt::type_id<Player>()) {
				// TODO: deep copy player, since it holds a held item entity
				storage.push(copied_entity, storage.value(entity));
			} else {
				storage.push(copied_entity, storage.value(entity));
			}
		}
		return copied_entity;
	}

	void set_lifetime(entt::entity entity, float time)
	{
		_registry.emplace_or_replace<Lifetime>(entity, time);
	}

	void destroy_immediately(entt::entity entity)
	{
		if (_registry.valid(entity)) {
			_registry.destroy(entity);
		}
	}

	void destroy_at_end_of_frame(entt::entity entity)
	{
		if (_registry.valid(entity)) {
			_entities_to_destroy_at_end_of_frame.insert(entity);
		}
	}

	bool valid(entt::entity entity)
	{
		return _registry.valid(entity);
	}

	void set_name(entt::entity entity, const std::string& name)
	{
		_registry.emplace_or_replace<Name>(entity, name);
	}

	void set_tag(entt::entity entity, Tag tag)
	{
		_registry.emplace_or_replace<Tag>(entity, tag);
	}

	std::string_view get_name(entt::entity entity)
	{
		const Name* name = _registry.try_get<const Name>(entity);
		if (!name) return "";
		return name->value;
	}

	Tag get_tag(entt::entity entity)
	{
		const Tag* tag = _registry.try_get<const Tag>(entity);
		if (!tag) return Tag::None;
		return *tag;
	}

	entt::entity find_entity_by_name(const std::string& name)
	{
		if (name.empty()) return entt::null;
		for (auto [entity, other_name] : _registry.view<const Name>().each()) {
			if (other_name.value == name) return entity;
		}
		return entt::null;
	}

	entt::entity find_entity_by_tag(Tag tag)
	{
		for (auto [entity, other_tag] : _registry.view<Tag>().each()) {
			if (other_tag == tag) return entity;
		}
		return entt::null;
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

	void set_bool(entt::entity entity, const std::string& name, bool value)
	{
		_registry.get_or_emplace<Properties>(entity).set_bool(name, value);
	}

	void set_float(entt::entity entity, const std::string& name, float value)
	{
		_registry.get_or_emplace<Properties>(entity).set_float(name, value);
	}

	void set_int(entt::entity entity, const std::string& name, int value)
	{
		_registry.get_or_emplace<Properties>(entity).set_int(name, value);
	}

	void set_string(entt::entity entity, const std::string& name, const std::string& value)
	{
		_registry.get_or_emplace<Properties>(entity).set_string(name, value);
	}

	void set_entity(entt::entity entity, const std::string& name, entt::entity value)
	{
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
