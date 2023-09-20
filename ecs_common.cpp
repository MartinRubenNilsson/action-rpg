#include "ecs_common.h"

namespace ecs
{
	extern entt::registry _registry;
	std::unordered_set<entt::entity> _entities_to_destroy;

	const tmx::Object* _try_get_object(entt::entity entity)
	{
		if (auto object = _registry.try_get<const tmx::Object*>(entity))
			return *object;
		return nullptr;
	}

	entt::entity find_entity_by_name(const std::string& name)
	{
		for (auto [entity, object] : _registry.view<const tmx::Object*>().each())
		{
			if (object->getName() == name)
				return entity;
		}
		return entt::null;
	}

	entt::entity find_entity_by_type(const std::string& type)
	{
		for (auto [entity, object] : _registry.view<const tmx::Object*>().each())
		{
			if (object->getType() == type)
				return entity;
		}
		return entt::null;
	}

	std::string get_name(entt::entity entity)
	{
		std::string name;
		if (auto object = _try_get_object(entity))
			name = object->getName();
		return name;
	}

	std::string get_type(entt::entity entity)
	{
		std::string type;
		if (auto object = _try_get_object(entity))
			type = object->getType();
		return type;
	}

	bool get_bool(entt::entity entity, const std::string& name, bool& value)
	{
		if (auto object = _try_get_object(entity))
		{
			for (const auto& prop : object->getProperties())
			{
				if (prop.getType() == tmx::Property::Type::Boolean &&
					prop.getName() == name)
				{
					value = prop.getBoolValue();
					return true;
				}
			}
		}
		return false;
	}

	bool get_float(entt::entity entity, const std::string& name, float& value)
	{
		if (auto object = _try_get_object(entity))
		{
			for (const auto& prop : object->getProperties())
			{
				if (prop.getType() == tmx::Property::Type::Float &&
					prop.getName() == name)
				{
					value = prop.getFloatValue();
					return true;
				}
			}
		}
		return false;
	}

	bool get_int(entt::entity entity, const std::string& name, int& value)
	{
		if (auto object = _try_get_object(entity))
		{
			for (const auto& prop : object->getProperties())
			{
				if (prop.getType() == tmx::Property::Type::Int &&
					prop.getName() == name)
				{
					value = prop.getIntValue();
					return true;
				}
			}
		}
		return false;
	}

	bool get_string(entt::entity entity, const std::string& name, std::string& value)
	{
		if (auto object = _try_get_object(entity))
		{
			for (const auto& prop : object->getProperties())
			{
				if (prop.getType() == tmx::Property::Type::String &&
					prop.getName() == name)
				{
					value = prop.getStringValue();
					return true;
				}
			}
		}
		return false;
	}

	bool get_entity(entt::entity entity, const std::string& name, entt::entity& value)
	{
		if (auto object = _try_get_object(entity))
		{
			for (const auto& prop : object->getProperties())
			{
				if (prop.getType() == tmx::Property::Type::Object &&
					prop.getName() == name)
				{
					value = (entt::entity)prop.getObjectValue();
					return true;
				}
			}
		}
		return false;
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
