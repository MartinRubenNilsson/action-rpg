#pragma once

namespace postprocessing
{
	// Calling render() applies the following effects in order:
	// 
	// 1. Shockwaves
	// 2. Lighting
	// 3. Screen transition
	// 4. Gaussian blur

	extern const size_t MAX_GAUSSIAN_BLUR_ITERATIONS;

	void update(float dt);
	void render(const Vector2f &camera_min, const Vector2f& camera_max);

	void add_shockwave(const Vector2f& position_ws); // ws = world space
	void set_darkness_intensity(float intensity); // 0 <= intensity <= 1
	void set_darkness_center(const Vector2f& position_ws); // ws = world space
	void set_screen_transition_progress(float progress); // -1 <= progress <= 1
	void set_gaussian_blur_iterations(size_t iterations); // Each Gaussian blur uses a 9x9 kernel
}

