#pragma once

class Properties; // Forward declaration

namespace ecs
{
	void update_common(float dt);

	// ENTITY CREATION/DESTRUCTION

	void clear();
	entt::entity create();
	entt::entity create(entt::entity hint);
	entt::entity deep_copy(entt::entity entity);
	void destroy_immediately(entt::entity entity);
	void destroy_at_end_of_frame(entt::entity entity);
	bool valid(entt::entity entity);

	// NAME AND CLASS

	void set_name(entt::entity entity, const std::string& name);
	void set_class(entt::entity entity, const std::string& class_);
	const std::string& get_name(entt::entity entity);
	const std::string& get_class(entt::entity entity);
	entt::entity find_entity_by_name(const std::string& name);
	entt::entity find_entity_by_class(const std::string& class_);

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
