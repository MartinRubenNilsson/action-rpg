#pragma once

namespace ecs
{
	struct LifeSpan { float value = 0.f; };

	bool has_name(entt::entity entity, const std::string& name);
	bool has_type(entt::entity entity, const std::string& type);
	void set_life_span(entt::entity entity, float life_span);
}