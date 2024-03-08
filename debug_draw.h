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

	struct Text
	{
		std::string string;
		sf::Vector2f position;
		float lifetime = 0.f;
	};

	void update(float dt);
	void render(sf::RenderTarget& target);

	void draw_line(const Line& line);
	void draw_text(const Text& text);
}

