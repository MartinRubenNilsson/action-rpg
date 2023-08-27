#pragma once

namespace ecs
{
	struct LifeSpan { float value = 0.f; };

	// Marks an entity to be destroyed at the end of the frame.
	// Calling this function is almost always preferable
	// to calling entt::registry::destroy directly.
	void mark_for_destruction(entt::entity entity);
	std::string get_name(entt::entity entity);
	std::string get_type(entt::entity entity);
	void set_life_span(entt::entity entity, float life_span);

	void update_life_spans(float dt);
	void destroy_marked_entities();
}