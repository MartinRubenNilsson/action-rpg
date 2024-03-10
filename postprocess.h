#pragma once

namespace postprocess
{
	void update(float dt);
	// Some postprocessing effects require intermediate render textures, and
	// these are pooled and reused for efficiency. This includes the texture
	// you pass to render(), so note that you may get a different texture back!
	void render(std::unique_ptr<sf::RenderTexture>& texture); 

	void create_shockwave(const sf::Vector2f& world_position);
}

