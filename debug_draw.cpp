#include "stdafx.h"
#include "debug_draw.h"
#include "fonts.h"

namespace debug
{
#ifdef _DEBUG
	std::vector<Line> _lines;
	std::vector<Text> _texts;

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
		sf::Vector2f view_center = target.getView().getCenter();
		sf::Vector2f view_size = target.getView().getSize();
		sf::FloatRect view_rect(view_center - view_size / 2.f, view_size);

		for (const Line& line : _lines) {
			//TODO: culling
			//TODO: batch into bigger vertex array
			//TODO: store static list of vertices
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

	void draw_line(const Line& line) {
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

