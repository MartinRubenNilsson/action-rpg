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

	void update(float dt);
	std::unique_ptr<sf::RenderTexture> render(std::unique_ptr<sf::RenderTexture> source);
}

