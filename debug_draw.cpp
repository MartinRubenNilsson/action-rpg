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

	ViewBounds _last_calculated_view_bounds{};
	std::vector<Line> _lines;
	std::vector<Text> _texts;

	bool _cull_line(const ViewBounds& bounds, const Line& line)
	{
		if (line.start.x < bounds.min_x && line.end.x < bounds.min_x) return true;
		if (line.start.x > bounds.max_x && line.end.x > bounds.max_x) return true;
		if (line.start.y < bounds.min_y && line.end.y < bounds.min_y) return true;
		if (line.start.y > bounds.max_y && line.end.y > bounds.max_y) return true;
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
	}

	void render(sf::RenderTarget& target)
	{
		const sf::Vector2f view_center = target.getView().getCenter();
		const sf::Vector2f view_size = target.getView().getSize();
		_last_calculated_view_bounds.min_x = view_center.x - view_size.x / 2.f;
		_last_calculated_view_bounds.min_y = view_center.y - view_size.y / 2.f;
		_last_calculated_view_bounds.max_x = view_center.x + view_size.x / 2.f;
		_last_calculated_view_bounds.max_y = view_center.y + view_size.y / 2.f;

		for (const Line& line : _lines) {
			if (_cull_line(_last_calculated_view_bounds, line))
				continue;
			sf::Vertex vertices[] = {
				sf::Vertex(line.start, line.color),
				sf::Vertex(line.end, line.color)
			};
			target.draw(vertices, 2, sf::Lines);
		}

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

	void draw_line(const Line& line)
	{
		if (line.lifetime <= 0.f && _cull_line(_last_calculated_view_bounds, line))
			return;
		_lines.push_back(line);
	}

	void draw_text(const Text& text) {
		_texts.push_back(text);
	}
#else
	void update(float) {}
	void render(sf::RenderTarget&) {}
	void draw_line(const Line&) {}
	void draw_text(const Text&) {}
#endif
}

