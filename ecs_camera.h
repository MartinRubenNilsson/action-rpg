#pragma once

// Math for Game Programmers: Juicing Your Cameras With Math
// https://www.youtube.com/watch?v=tu-Qe66AvtY

namespace ecs
{
	// All camera views use world space units (meters), not pixels!

	sf::View get_default_camera_view();
	sf::View get_current_camera_view();
	
	// view:               The camera view, which determines the camera's position, rotation, and zoom.
	// _shaky_view:        The camera view with shake applied to it. Read-only!
	// priority:           The camera with the highest priority is considered the "main" camera.
	// follow:             If not null, the camera will center its view on this entity.
	// confining_rect:     The camera view will be confined to this rect. If the width or height is zero,
	//                     the view will not be confined in that direction. If instead the width or height
	//                     is less than the view's, the view will be centered on the rect in that direction.
	// shake_amplitude:    The amplitude of the camera shake.
	// shake_frequency:    The frequency of the camera shake.
	// trauma:			   A number between 0 and 1 that determines the total shake amplitude by the formula:
	//					   total_shake_amplitude = shake_amplitude * trauma^2. It decays linearly over time.
	// trauma_decay:       The trauma decays by this amount per second.
	struct Camera
	{
		sf::View view = get_default_camera_view();
		sf::View _shaky_view;
		float priority = 0.f;
		entt::entity follow = entt::null;
		sf::FloatRect confining_rect;
		float shake_amplitude = 0.5f;
		float shake_frequency = 10.f;
		float trauma = 0.f;
		float trauma_decay = 2.f;
	};

	void update_cameras(float dt);
}
