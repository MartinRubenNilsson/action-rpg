#pragma once

namespace postprocess
{
	struct Shockwave
	{
		sf::Vector2f center; // in target coordinates
		float force = 0.f;
		float size = 0.f;
		float thickness = 0.f;
	};

	extern std::vector<Shockwave> shockwaves;

	void render_copy(sf::RenderTarget& target, const sf::Texture& source);
	void render_shockwave(sf::RenderTarget& target, const sf::Texture& source, const Shockwave& shockwave);
}

