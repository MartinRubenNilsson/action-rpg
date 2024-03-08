#pragma once

namespace debug
{
	struct Line
	{
		sf::Vector2f start;
		sf::Vector2f end;
		sf::Color color = sf::Color::White;
		float lifetime = 0.f;
	};

	void update(float dt);
	void render(sf::RenderTarget& target);

	void draw_line(const Line& line);
}

