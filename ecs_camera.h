#pragma once

// Math for Game Programmers: Juicing Your Cameras With Math
// https://www.youtube.com/watch?v=tu-Qe66AvtY

namespace ecs
{
	// All camera views use world space units (meters), not pixels!

	extern const sf::View DEFAULT_CAMERA_VIEW;
	
	// view:               The camera view, which determines the camera's position, rotation, and zoom.
	// _shaky_view:        The camera view with shake applied to it. Read-only!
	// follow:             If not null, the camera will center its view on this entity.
	// confining_rect:     The camera view will be confined to this rect. If the width or height is zero,
	//                     the view will not be confined along that axis, and if the width or height is
	//                     smaller than that of the view, the view will be centered on the rect.
	// shake_amplitude:    The amplitude of the camera shake.
	// shake_frequency:    The frequency of the camera shake.
	// trauma:			   A number between 0 and 1 that determines the total shake amplitude by the formula:
	//					   total_shake_amplitude = shake_amplitude * trauma^2. It decays linearly over time.
	// trauma_decay:       The trauma decreases by this amount per second.

	struct Camera
	{
		sf::View view = DEFAULT_CAMERA_VIEW;
		sf::View _shaky_view; // Read-only!
		entt::entity follow = entt::null;
		sf::FloatRect confining_rect;
		float shake_amplitude = 0.5f;
		float shake_frequency = 10.f;
		float trauma = 0.f;
		float trauma_decay = 2.f;
	};

	void update_cameras(float dt);
	const sf::View& get_active_camera_view(); // Hard-cuts when activating new camera.
	const sf::View& get_blended_camera_view(); // Smoothly transitions between cameras.

	void emplace_camera(entt::entity entity, const Camera& camera);
	// Copies the camera to a new entity, removes it from the old entity, and returns the new entity.
	entt::entity detach_camera(entt::entity entity);
	bool activate_camera(entt::entity entity, bool hard_cut = false);
	bool add_camera_trauma(entt::entity entity, float trauma);
}
