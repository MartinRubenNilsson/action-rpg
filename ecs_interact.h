#pragma once

namespace ecs
{
	// AREA-BASED INTERACTION FUNCTIONS

	void interact_in_box(const Vector2f& box_min, const Vector2f& box_max);

	// UNIVERSAL INTERACTION FUNCTION

	// Delegates to the appropriate function based on the entity's class (e.g. chest, lever, etc.)
	void interact(entt::entity entity);

	// CLASS-SPECIFIC INTERACT FUNCTIONS

	void interact_with_chest(entt::entity entity);
}