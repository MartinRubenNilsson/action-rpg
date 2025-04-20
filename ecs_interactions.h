#pragma once

namespace ecs {
	using InteractionCallback = void(*)(entt::entity entity);

	void set_interaction_callback(entt::entity entity, InteractionCallback callback);
	InteractionCallback get_interaction_callback(entt::entity entity);

	// Calls the entity's interaction callback, if it exists.
	void interact_with(entt::entity entity);

	// Calls the interaction callback for all entities that intersect the given box.
	void interact_with_all_entities_in_box(const Vector2f& box_min, const Vector2f& box_max);
}