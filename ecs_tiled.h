#pragma once

namespace ecs
{
	entt::entity find_entity_by_name(const std::string& name);
	entt::entity find_entity_by_class(const std::string& class_);
	std::string get_name(entt::entity entity);
	std::string get_class(entt::entity entity);
	void set_bool(entt::entity entity, const std::string& name, bool value);
	bool get_bool(entt::entity entity, const std::string& name, bool& value);
	void set_float(entt::entity entity, const std::string& name, float value);
	bool get_float(entt::entity entity, const std::string& name, float& value);
	void set_int(entt::entity entity, const std::string& name, int value);
	bool get_int(entt::entity entity, const std::string& name, int& value);
	void set_string(entt::entity entity, const std::string& name, const std::string& value);
	bool get_string(entt::entity entity, const std::string& name, std::string& value);
	void set_entity(entt::entity entity, const std::string& name, entt::entity value);
	bool get_entity(entt::entity entity, const std::string& name, entt::entity& value);
}