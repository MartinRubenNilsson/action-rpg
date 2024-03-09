#include "stdafx.h"
#include "debug_draw.h"
#include "fonts.h"

namespace debug
{
#ifdef _DEBUG
	struct ViewBounds
	{
		float min_x = 0.f;
		float min_y = 0.f;
		float max_x = 0.f;
		float max_y = 0.f;
	};

	struct Line
	{
		sf::Vector2f p1;
		sf::Vector2f p2;
		sf::Color color = sf::Color::White;
		float lifetime = 0.f;
	};

	struct Circle
	{
		sf::Vector2f center;
		float radius;
		sf::Color color = sf::Color::White;
		float lifetime = 0.f;
	};

	struct Text
	{
		std::string string;
		sf::Vector2f position;
		float lifetime = 0.f;
	};

	// Culling will use the view bounds from last render call,
	// leading to one frame of lag, but it's not a big deal.
	ViewBounds _last_calculated_view_bounds{};
	std::vector<Line> _lines;
	std::vector<Text> _texts;
	std::vector<Circle> _circles;

	bool _cull_line(const ViewBounds& bounds, const sf::Vector2f& p1, const sf::Vector2f& p2)
	{
		if (p1.x < bounds.min_x && p2.x < bounds.min_x) return true;
		if (p1.x > bounds.max_x && p2.x > bounds.max_x) return true;
		if (p1.y < bounds.min_y && p2.y < bounds.min_y) return true;
		if (p1.y > bounds.max_y && p2.y > bounds.max_y) return true;
		return false;
	}

	bool _cull_circle(const ViewBounds& bounds, const sf::Vector2f& center, float radius)
	{
		if (center.x + radius < bounds.min_x) return true;
		if (center.x - radius > bounds.max_x) return true;
		if (center.y + radius < bounds.min_y) return true;
		if (center.y - radius > bounds.max_y) return true;
		return false;
	}

	template <typename T>
	void _update(std::vector<T>& vec, float dt)
	{
		size_t size = vec.size();
		for (size_t i = size; i--;) {
			vec[i].lifetime -= dt;
			if (vec[i].lifetime > 0.f) continue;
			--size;
			if (i == size) continue;
			if constexpr (std::is_trivially_copyable_v<T>)
				std::memcpy(&vec[i], &vec[size], sizeof(T));
			else
				vec[i] = std::move(vec[size]);
		}
		vec.resize(size);
	}

	void update(float dt)
	{
		_update(_lines, dt);
		_update(_texts, dt);
		_update(_circles, dt);
	}

	void render(sf::RenderTarget& target)
	{
		const sf::Vector2f view_center = target.getView().getCenter();
		const sf::Vector2f view_size = target.getView().getSize();
		_last_calculated_view_bounds.min_x = view_center.x - view_size.x / 2.f;
		_last_calculated_view_bounds.min_y = view_center.y - view_size.y / 2.f;
		_last_calculated_view_bounds.max_x = view_center.x + view_size.x / 2.f;
		_last_calculated_view_bounds.max_y = view_center.y + view_size.y / 2.f;

		// RENDER LINES

		for (const Line& line : _lines) {
			if (_cull_line(_last_calculated_view_bounds, line.p1, line.p2))
				continue;
			sf::Vertex vertices[] = {
				sf::Vertex(line.p1, line.color),
				sf::Vertex(line.p2, line.color)
			};
			target.draw(vertices, 2, sf::Lines);
		}

		// RENDER CIRCLES
		{
			sf::CircleShape shape;
			shape.setFillColor(sf::Color::Transparent);
			shape.setOutlineThickness(0.25f);
			for (const Circle& circle : _circles) {
				if (_cull_circle(_last_calculated_view_bounds, circle.center, circle.radius))
					continue;
				shape.setRadius(circle.radius);
				shape.setPosition(circle.center - sf::Vector2f{ circle.radius, circle.radius });
				shape.setOutlineColor(circle.color);
				target.draw(shape);
			}
		}

		// RENDER TEXT

		std::shared_ptr<sf::Font> font = fonts::get("Helvetica");
		if (!font) return;

		sf::Text text{};
		text.setFont(*font);
		text.setCharacterSize(48);
		text.setScale(0.1f, 0.1f);
		text.setFillColor(sf::Color::White);
		text.setOutlineColor(sf::Color::Black);
		text.setOutlineThickness(2.f);

		for (const Text& t : _texts) {
			text.setString(t.string);
			text.setPosition(t.position);
			text.setOrigin(text.getLocalBounds().width / 2.f, text.getLocalBounds().height / 2.f);
			//TODO: culling
			target.draw(text);
		}
	}

	void draw_line(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Color& color, float lifetime)
	{
		if (lifetime > 0.f || !_cull_line(_last_calculated_view_bounds, p1, p2))
			_lines.emplace_back(p1, p2, color, lifetime);
	}

	void draw_circle(const sf::Vector2f& center, float radius, const sf::Color& color, float lifetime)
	{
		if (lifetime > 0.f || !_cull_circle(_last_calculated_view_bounds, center, radius))
			_circles.emplace_back(center, radius, color, lifetime);
	}

	void draw_text(const std::string& string, const sf::Vector2f& position, float lifetime) {
		//TODO: culling
		_texts.emplace_back(string, position, lifetime);
	}
#else
	void update(float) {}
	void render(sf::RenderTarget&) {}
	void draw_line(const Line&) {}
	void draw_text(const Text&) {}
#endif
}

