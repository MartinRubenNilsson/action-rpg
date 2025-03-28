#pragma once
#include "ecs_tags.h"

class Properties; // Forward declaration

namespace ecs
{
	struct Lifetime
	{
		float time = 0.f;
	};

	void update_lifetimes(float dt);
	void destroy_entities_to_be_destroyed_at_end_of_frame();

	// ENTITY CREATION/DESTRUCTION

	void clear();
	entt::entity create();
	entt::entity create(entt::entity hint);
	entt::entity deep_copy(entt::entity entity);
	void set_lifetime(entt::entity entity, float time);
	void destroy_immediately(entt::entity entity);
	void destroy_at_end_of_frame(entt::entity entity);
	bool valid(entt::entity entity);

	// NAME AND TAG

	void set_name(entt::entity entity, const std::string& name);
	void set_tag(entt::entity entity, Tag tag);
	std::string_view get_name(entt::entity entity);
	Tag get_tag(entt::entity entity);
	entt::entity find_entity_by_name(const std::string& name);
	entt::entity find_entity_by_tag(Tag tag);

	// PROPERTIES

	void set_properties(entt::entity entity, const Properties& properties);
	bool get_properties(entt::entity entity, Properties& properties);
	void set_bool(entt::entity entity, const std::string& name, bool value);
	bool get_bool(entt::entity entity, const std::string& name, bool& value);
	void set_int(entt::entity entity, const std::string& name, int value);
	bool get_int(entt::entity entity, const std::string& name, int& value);
	void set_string(entt::entity entity, const std::string& name, const std::string& value);
	bool get_string(entt::entity entity, const std::string& name, std::string& value);
	void set_float(entt::entity entity, const std::string& name, float value);
	bool get_float(entt::entity entity, const std::string& name, float& value);
	void set_entity(entt::entity entity, const std::string& name, entt::entity value);
	bool get_entity(entt::entity entity, const std::string& name, entt::entity& value);
}
