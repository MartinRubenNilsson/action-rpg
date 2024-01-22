#pragma once

namespace tiled
{
	struct Object; //TODO: remove
}

namespace ecs
{
	void emplace_object(entt::entity entity, const tiled::Object* object); //TODO: remove

	entt::entity find_entity_by_name(const std::string& name);
	entt::entity find_entity_by_class(const std::string& class_);
	void set_name(entt::entity entity, const std::string& name);
	void set_class(entt::entity entity, const std::string& class_);
	std::string get_name(entt::entity entity);
	std::string get_class(entt::entity entity);
	bool get_bool(entt::entity entity, const std::string& name, bool& value);
	bool get_float(entt::entity entity, const std::string& name, float& value);
	bool get_int(entt::entity entity, const std::string& name, int& value);
	bool get_string(entt::entity entity, const std::string& name, std::string& value);
	bool get_entity(entt::entity entity, const std::string& name, entt::entity& value);
}
