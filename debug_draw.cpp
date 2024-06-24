#include "stdafx.h"
#include "debug_draw.h"
#include "graphics.h"
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
		unsigned int count = 0;
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
	std::vector<graphics::Vertex> _vertices;

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
				memcpy(&vec[i], &vec[size], sizeof(T));
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

	void _render_lines()
	{
		const size_t line_count = _lines.size();
		if (line_count == 0) return;
		_vertices.resize(line_count * 2);
		for (size_t l = 0; l < line_count; ++l) {
			const Line& line = _lines[l];
			if (_cull_line(_last_calculated_view_bounds, line.p1, line.p2))
				continue;
			_vertices[2 * l + 0].position = line.p1;
			_vertices[2 * l + 0].color = line.color;
			_vertices[2 * l + 1].position = line.p2;
			_vertices[2 * l + 1].color = line.color;
		}
		graphics::draw_lines(_vertices.data(), (unsigned int)_vertices.size());
	}

	void _render_boxes()
	{
		if (_boxes.empty()) return;
		graphics::Vertex vertices[4];
		for (const Box& box : _boxes) {
			if (_cull_box(_last_calculated_view_bounds, box.min, box.max))
				continue;
			vertices[0].position = { box.min.x, box.min.y };
			vertices[1].position = { box.max.x, box.min.y };
			vertices[2].position = { box.max.x, box.max.y };
			vertices[3].position = { box.min.x, box.max.y };
			vertices[0].color = box.color;
			vertices[1].color = box.color;
			vertices[2].color = box.color;
			vertices[3].color = box.color;
			graphics::draw_line_loop(vertices, 4);
		}
	}

	void _render_polygons()
	{
		if (_polygons.empty()) return;
		graphics::Vertex vertices[MAX_POLYGON_VERTICES + 1];
		for (const Polygon& polygon : _polygons) {
			if (_cull_polygon(_last_calculated_view_bounds, polygon.points, polygon.count))
				continue;
			for (size_t i = 0; i < polygon.count; ++i) {
				vertices[i].position = polygon.points[i];
				vertices[i].color = polygon.color;
			}
			vertices[polygon.count].position = polygon.points[0];
			vertices[polygon.count].color = polygon.color;
			graphics::draw_line_strip(vertices, polygon.count + 1);
		}
	}

	void _render_circles()
	{
		if (_circles.empty()) return;
		constexpr unsigned int VERTEX_COUNT = 32;
		constexpr float ANGLE_STEP = 6.283185307f / VERTEX_COUNT;
		graphics::Vertex vertices[VERTEX_COUNT];
		for (const Circle& circle : _circles) {
			if (_cull_circle(_last_calculated_view_bounds, circle.center, circle.radius))
				continue;
			for (unsigned int i = 0; i < VERTEX_COUNT; ++i) {
				const float angle = i * ANGLE_STEP;
				vertices[i].position = circle.center + circle.radius * sf::Vector2f{ std::cos(angle), std::sin(angle) };
				vertices[i].color = circle.color;
			}
			graphics::draw_line_loop(vertices, VERTEX_COUNT);
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

	void render(const sf::Vector2f& camera_min, const sf::Vector2f& camera_max)
	{
		_last_calculated_view_bounds.min_x = camera_min.x;
		_last_calculated_view_bounds.min_y = camera_min.y;
		_last_calculated_view_bounds.max_x = camera_max.x;
		_last_calculated_view_bounds.max_y = camera_max.y;
		_render_lines();
		_render_boxes();
		_render_polygons();
		_render_circles();
		//_render_texts(target);
		//target.resetGLStates();
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

	void draw_polygon(const sf::Vector2f* points, unsigned int count, const sf::Color& color, float lifetime)
	{
		count = std::min(count, MAX_POLYGON_VERTICES);
		if (count < 3) return;
		if (lifetime <= 0.f && _cull_polygon(_last_calculated_view_bounds, points, count)) return;
		Polygon& polygon = _polygons.emplace_back();
		memcpy(polygon.points, points, count * sizeof(sf::Vector2f));
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
	void render_sprites_in_render_queue(sf::RenderTarget&) {}
	void draw_line(const Line&) {}
	void draw_box(const Box&) {}
	void draw_polygon(const Polygon&) {}
	void draw_circle(const Circle&) {}
	void draw_text(const Text&) {}
#endif
}

