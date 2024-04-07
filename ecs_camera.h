#pragma once

// Math for Game Programmers: Juicing Your Cameras With Math
// https://www.youtube.com/watch?v=tu-Qe66AvtY

namespace ecs
{
	// view:               The camera view, which determines the camera's position, rotation, and zoom.
	// _shaken_view:       The camera view with shake applied to it. Read-only!
	// follow:             If not null, the camera will center its view on this entity.
	// confines_min/max:   The camera view will be confined to this box. If the width or height is zero,
	//                     the view will not be confined along that axis, and if the width or height is
	//                     smaller than that of the view, the view will be centered on the box.
	// shake_amplitude:    The amplitude of the camera shake.
	// shake_frequency:    The frequency of the camera shake.
	// trauma:			   A number between 0 and 1 that determines the total shake amplitude by the formula:
	//					   total_shake_amplitude = shake_amplitude * trauma^2. It decays linearly over time.
	// trauma_decay:       The trauma decreases by this amount per second.

	struct CameraView
	{
		sf::Vector2f size = { 320, 180 };
		sf::Vector2f center;
	};

	struct Camera
	{
		CameraView view;
		CameraView _shaken_view; // Read-only!
		entt::entity follow = entt::null;
		sf::Vector2f confines_min;
		sf::Vector2f confines_max;
		float shake_amplitude = 6.f;
		float shake_frequency = 10.f;
		float trauma = 0.f;
		float trauma_decay = 1.5f;
	};

	void update_cameras(float dt);

	const CameraView& get_active_camera_view(); // Hard-cuts when activating new camera.
	const CameraView& get_blended_camera_view(); // Smoothly transitions between cameras.

	void emplace_camera(entt::entity entity, const Camera& camera);
	// Copies the camera to a new entity, removes it from the old entity, and returns the new entity.
	entt::entity detach_camera(entt::entity entity);
	bool activate_camera(entt::entity entity, bool hard_cut = false);
	bool add_trauma_to_camera(entt::entity entity, float trauma);
	bool add_trauma_to_active_camera(float trauma);
}
