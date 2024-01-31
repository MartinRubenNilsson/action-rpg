#pragma once

namespace postprocess
{
	void copy(sf::RenderTarget& target, const sf::RenderTexture& source);
	void shockwave(sf::RenderTarget& target, const sf::RenderTexture& source, float time, const sf::Vector2f& center);
}

