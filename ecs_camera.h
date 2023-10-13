#pragma once

namespace ecs
{
	struct Camera
	{
		// Uses world space units (meters), not pixels.
		sf::View view = { { 0.f, 0.f }, { VIEW_WIDTH, VIEW_HEIGHT } };
		float priority = 0.f;
		entt::entity follow_target = entt::null;
		sf::FloatRect confining_box; // If width or height is 0, that dimension is not constrained.
	};

	void update_cameras(float dt);

	const sf::View& get_current_camera_view();
}
