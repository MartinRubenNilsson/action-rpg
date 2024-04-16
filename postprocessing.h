#pragma once

namespace postprocessing
{
	// Calling render() applies the following effects in order:
	// 
	// 1. Shockwaves
	// 2. Darkness
	// 3. Screen transition
	// 4. Gaussian blur
	// 
	// Some postprocessing effects use intermediate render textures, which
	// are pooled and reused for efficiency. This includes the texture you
	// pass to render(), meaning that you may get a different texture back!

	extern const size_t MAX_GAUSSIAN_BLUR_ITERATIONS;

	void update(float dt);
	void render(std::unique_ptr<sf::RenderTexture>& texture); 

	void set_pixel_scale(float scale);
	void create_shockwave(const sf::Vector2f& position_in_world_space);
	void set_darkness_intensity(float intensity); // 0 <= intensity <= 1
	void set_darkness_center(const sf::Vector2f& position_in_world_space);
	void set_screen_transition_progress(float progress); // -1 <= progress <= 1
	void set_gaussian_blur_iterations(size_t iterations); // Each Gaussian blur uses a 9x9 kernel
}

