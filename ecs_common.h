#pragma once

namespace ecs
{
	struct LifeSpan { float value = 0.f; };

	std::string get_name(entt::entity entity);
	std::string get_type(entt::entity entity);

	bool get_bool(entt::entity entity, std::string& name, bool& value);
	bool get_float(entt::entity entity, std::string& name, float& value);
	bool get_int(entt::entity entity, std::string& name, int& value);
	bool get_string(entt::entity entity, std::string& name, std::string& value);
	bool get_entity(entt::entity entity, std::string& name, entt::entity& value);

	void set_life_span(entt::entity entity, float life_span);

	// Marks an entity to be destroyed at the end of the frame.
	// Calling this function is almost always preferable
	// to calling entt::registry::destroy directly.
	void mark_for_destruction(entt::entity entity);

	void update_common(float dt);
}