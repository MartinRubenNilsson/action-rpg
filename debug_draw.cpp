#include "stdafx.h"
#include "debug_draw.h"

namespace debug
{
	std::vector<Line> _lines;

	template <typename T>
	void _update(std::vector<T>& vec, float dt)
	{
		size_t size = vec.size();
		for (size_t i = 0; i < size;) {
			vec[i].lifetime -= dt;
			if (vec[i].lifetime <= 0.f) {
				if (i != size - 1) {
					if constexpr (std::is_trivially_copyable_v<T>)
						std::memcpy(&vec[i], &vec[size - 1], sizeof(T));
					else
						vec[i] = std::move(vec[size - 1]);
				}
				--size;
			} else {
				++i;
			}
		}
		vec.resize(size);
	}

	void update(float dt)
	{
		_update(_lines, dt);
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
	}

	void draw_line(const Line& line) {
		_lines.push_back(line);
	}
}

