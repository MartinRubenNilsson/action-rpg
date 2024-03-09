#pragma once

namespace debug
{
	const size_t MAX_POLYGON_VERTICES = 8;

	void update(float dt);
	void render(sf::RenderTarget& target);

	void draw_line(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Color& color = sf::Color::White, float lifetime = 0.f);
	void draw_rect(const sf::FloatRect& rect, const sf::Color& color = sf::Color::White, float lifetime = 0.f);
	void draw_polygon(const sf::Vector2f* points, size_t count, const sf::Color& color = sf::Color::White, float lifetime = 0.f);
	void draw_circle(const sf::Vector2f& center, float radius, const sf::Color& color = sf::Color::White, float lifetime = 0.f);
	void draw_text(const std::string& string, const sf::Vector2f& position, float lifetime = 0.f);
}

