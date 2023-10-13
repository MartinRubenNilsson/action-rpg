#pragma once

namespace ecs
{
	// IMPORTANT: Camera views use world space units (meters), not pixels!

	sf::View get_default_camera_view();

	struct Camera
	{
		sf::View view = get_default_camera_view();
		float priority = 0.f; // The highest priority camera is used as the current camera.
		entt::entity follow = entt::null;
		sf::FloatRect confining_box; // If width or height is 0, that dimension is not constrained.

		// TODO: impulse/noise to shake the camera.
	};

	void update_cameras(float dt);

	const sf::View& get_current_camera_view();
}
