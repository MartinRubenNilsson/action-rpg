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

	struct Box
	{
		sf::Vector2f min;
		sf::Vector2f max;
		sf::Color color = sf::Color::White;
		float lifetime = 0.f;
	};

	struct Polygon
	{
		sf::Vector2f points[MAX_POLYGON_VERTICES];
		size_t count = 0;
		sf::Color color = sf::Color::White;
		float lifetime = 0.f;
	};

	struct Circle
	{
		sf::Vector2f center;
		float radius = 0.f;
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
	std::vector<Box> _boxes;
	std::vector<Polygon> _polygons;
	std::vector<Circle> _circles;
	std::vector<Text> _texts;


	bool _cull_line(const ViewBounds& bounds, const sf::Vector2f& p1, const sf::Vector2f& p2)
	{
		if (p1.x < bounds.min_x && p2.x < bounds.min_x) return true;
		if (p1.x > bounds.max_x && p2.x > bounds.max_x) return true;
		if (p1.y < bounds.min_y && p2.y < bounds.min_y) return true;
		if (p1.y > bounds.max_y && p2.y > bounds.max_y) return true;
		return false;
	}

	bool _cull_box(const ViewBounds& bounds, const sf::Vector2f& min, const sf::Vector2f& max)
	{
		if (max.x < bounds.min_x) return true;
		if (min.x > bounds.max_x) return true;
		if (max.y < bounds.min_y) return true;
		if (min.y > bounds.max_y) return true;
		return false;
	}

	bool _cull_polygon(const ViewBounds& bounds, const sf::Vector2f* points, size_t count)
	{
		if (count < 3) return true;
		sf::Vector2f min = points[0];
		sf::Vector2f max = points[0];
		for (size_t i = 1; i < count; ++i) {
			min.x = std::min(min.x, points[i].x);
			min.y = std::min(min.y, points[i].y);
			max.x = std::max(max.x, points[i].x);
			max.y = std::max(max.y, points[i].y);
		}
		return _cull_box(bounds, min, max);
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
		_update(_boxes, dt);
		_update(_polygons, dt);
		_update(_circles, dt);
		_update(_texts, dt);
	}

	void _render_lines(sf::RenderTarget& target)
	{
		if (_lines.empty()) return;
		sf::Vertex vertices[2];
		for (const Line& line : _lines) {
			if (_cull_line(_last_calculated_view_bounds, line.p1, line.p2))
				continue;
			vertices[0].position = line.p1;
			vertices[1].position = line.p2;
			vertices[0].color = line.color;
			vertices[1].color = line.color;
			target.draw(vertices, 2, sf::Lines);
		}
	}

	void _render_boxes(sf::RenderTarget& target)
	{
		if (_boxes.empty()) return;
		sf::Vertex vertices[5];
		for (const Box& box : _boxes) {
			if (_cull_box(_last_calculated_view_bounds, box.min, box.max))
				continue;
			vertices[0].position = { box.min.x, box.min.y };
			vertices[1].position = { box.max.x, box.min.y };
			vertices[2].position = { box.max.x, box.max.y };
			vertices[3].position = { box.min.x, box.max.y };
			vertices[4].position = { box.min.x, box.min.y };
			for (size_t i = 0; i < 5; ++i)
				vertices[i].color = box.color;
			target.draw(vertices, 5, sf::LineStrip);
		}
	}

	void _render_polygons(sf::RenderTarget& target)
	{
		if (_polygons.empty()) return;
		sf::Vertex vertices[MAX_POLYGON_VERTICES + 1];
		for (const Polygon& polygon : _polygons) {
			if (_cull_polygon(_last_calculated_view_bounds, polygon.points, polygon.count))
				continue;
			for (size_t i = 0; i < polygon.count; ++i) {
				vertices[i].position = polygon.points[i];
				vertices[i].color = polygon.color;
			}
			vertices[polygon.count].position = polygon.points[0];
			vertices[polygon.count].color = polygon.color;
			target.draw(vertices, polygon.count + 1, sf::LineStrip);
		}
	}

	void _render_circles(sf::RenderTarget& target)
	{
		if (_circles.empty()) return;
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

	void _render_texts(sf::RenderTarget& target)
	{
		if (_texts.empty()) return;
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
			//TODO: culling
			text.setString(t.string);
			text.setPosition(t.position);
			text.setOrigin(text.getLocalBounds().width / 2.f, text.getLocalBounds().height / 2.f);
			target.draw(text);
		}
	}

	void render(sf::RenderTarget& target)
	{
		const sf::Vector2f view_center = target.getView().getCenter();
		const sf::Vector2f view_size = target.getView().getSize();
		_last_calculated_view_bounds.min_x = view_center.x - view_size.x / 2.f;
		_last_calculated_view_bounds.min_y = view_center.y - view_size.y / 2.f;
		_last_calculated_view_bounds.max_x = view_center.x + view_size.x / 2.f;
		_last_calculated_view_bounds.max_y = view_center.y + view_size.y / 2.f;

		_render_lines(target);
		_render_boxes(target);
		_render_polygons(target);
		_render_circles(target);
		_render_texts(target);
	}

	void draw_line(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Color& color, float lifetime)
	{
		if (lifetime <= 0.f && _cull_line(_last_calculated_view_bounds, p1, p2)) return;
		_lines.emplace_back(p1, p2, color, lifetime);
	}

	void draw_box(const sf::Vector2f& min, const sf::Vector2f& max, const sf::Color& color, float lifetime)
	{
		if (lifetime <= 0.f && _cull_box(_last_calculated_view_bounds, min, max)) return;
		_boxes.emplace_back(min, max, color, lifetime);
	}

	void draw_polygon(const sf::Vector2f* points, size_t count, const sf::Color& color, float lifetime)
	{
		count = std::min(count, MAX_POLYGON_VERTICES);
		if (count < 3) return;
		if (lifetime <= 0.f && _cull_polygon(_last_calculated_view_bounds, points, count)) return;
		Polygon& polygon = _polygons.emplace_back();
		std::memcpy(polygon.points, points, count * sizeof(sf::Vector2f));
		polygon.count = count;
		polygon.color = color;
		polygon.lifetime = lifetime;
	}

	void draw_circle(const sf::Vector2f& center, float radius, const sf::Color& color, float lifetime)
	{
		if (lifetime <= 0.f && _cull_circle(_last_calculated_view_bounds, center, radius)) return;
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
	void draw_box(const Box&) {}
	void draw_polygon(const Polygon&) {}
	void draw_circle(const Circle&) {}
	void draw_text(const Text&) {}
#endif
}

