#pragma once

namespace shapes
{
	const unsigned int MAX_POLYGON_VERTICES = 8;

	void update_lifetimes(float dt);
	void render(const sf::Vector2f& camera_min, const sf::Vector2f& camera_max);

	void add_line_to_render_queue(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Color& color = sf::Color::White, float lifetime = 0.f);
	void add_box_to_render_queue(const sf::Vector2f& min, const sf::Vector2f& max, const sf::Color& color = sf::Color::White, float lifetime = 0.f);
	void add_polygon_to_render_queue(const sf::Vector2f* points, unsigned int count, const sf::Color& color = sf::Color::White, float lifetime = 0.f);
	void add_circle_to_render_queue(const sf::Vector2f& center, float radius, const sf::Color& color = sf::Color::White, float lifetime = 0.f);
}

