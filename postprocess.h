#pragma once

namespace postprocess
{
	void update(float dt);
	std::unique_ptr<sf::RenderTexture> render(std::unique_ptr<sf::RenderTexture> source);

	void create_shockwave(const sf::Vector2f& center);
}

