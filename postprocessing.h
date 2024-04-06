#pragma once

namespace postprocessing
{
	extern const size_t MAX_GAUSSIAN_BLUR_ITERATIONS;

	void update(float dt);

	// Some postprocessing effects use intermediate render textures, which
	// are pooled and reused for efficiency. This includes the texture you
	// pass to render(), so note that you may get a different texture back!
	void render(std::unique_ptr<sf::RenderTexture>& texture); 

	void create_shockwave(const sf::Vector2f& world_position);
	void set_gaussian_blur_iterations(size_t iterations); // Each Gaussian blur uses a 9x9 kernel
}

