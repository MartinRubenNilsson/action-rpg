#pragma once

// Math for Game Programmers: Juicing Your Cameras With Math
// https://www.youtube.com/watch?v=tu-Qe66AvtY

namespace ecs
{
	// All camera views use world space units (meters), not pixels!

	sf::View get_default_camera_view();
	sf::View get_current_camera_view();
	
	// priority:       The camera with the highest priority is considered the "main" camera.
	// follow:         If not null, the camera will center its view on this entity.
	// confining_rect: The camera view will be confined to this rect. If the width or height is non-positive,
	//                 the view will not be confined in that direction. If instead the width or height
	//                 is less than the view's, the view will be centered on the rect in that direction.
	// view:           The camera view, which determines the camera's position, rotation, and zoom.
	// _shaky_view:    The camera view with shake applied to it. Read-only!
	struct Camera
	{
		float priority = 0.f;
		entt::entity follow = entt::null;
		sf::FloatRect confining_rect;
		sf::View view = get_default_camera_view();
		sf::View _shaky_view;
	};

	void update_cameras(float dt);
}
