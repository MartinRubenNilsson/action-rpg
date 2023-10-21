#pragma once

namespace ecs
{
	struct Name { std::string value; };
	struct Type { std::string value; };

	struct Property
	{
		std::string name;
		std::variant<bool, float, int, std::string, entt::entity> value;
	};

	struct Properties { std::vector<Property> properties; };

	entt::entity find_entity_by_name(const std::string& name);
	entt::entity find_entity_by_type(const std::string& type);

	void set_name(entt::entity entity, const std::string& name);
	std::string get_name(entt::entity entity);
	void set_type(entt::entity entity, const std::string& type);
	std::string get_type(entt::entity entity);

	void set_properties(entt::entity entity, const Properties& properties);
	bool get_properties(entt::entity entity, Properties& properties);
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

	// Marks an entity to be destroyed at the end of the frame.
	// Calling this function is almost always preferable
	// to calling entt::registry::destroy directly.
	void mark_for_destruction(entt::entity entity);

	void update_common(float dt);
}