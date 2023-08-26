#pragma once

namespace ecs
{
	struct Name { std::string value; };
	struct Type { std::string value; }; // Same as "class" in the Tiled editor.
	struct LifeSpan { float value = 0.f; };

	void set_name(entt::entity entity, const std::string& name);
	bool has_name(entt::entity entity, const std::string& name);
	void set_type(entt::entity entity, const std::string& type);
	bool has_type(entt::entity entity, const std::string& type);
	void set_life_span(entt::entity entity, float life_span);
}