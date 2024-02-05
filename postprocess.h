#pragma once

namespace postprocess
{
	void copy(sf::RenderTarget& target, const sf::Texture& source);
	void shockwave(sf::RenderTarget& target, const sf::Texture& source,
		const sf::Vector2f& center, float force, float size, float thickness);
}

