#include "window.h"

namespace window
{
	sf::RenderWindow _window;
	uint32_t _scale = 5;

	sf::RenderWindow& create()
	{
		sf::VideoMode video_mode(
			_scale * WINDOW_VIEW_WIDTH,
			_scale * WINDOW_VIEW_HEIGHT);
		std::string title = "Untitled RPG";
		sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
		_window.create(video_mode, title, style);

		sf::View view(sf::FloatRect(
			0, 0, WINDOW_VIEW_WIDTH, WINDOW_VIEW_HEIGHT));
		_window.setView(view);

		return _window;
	}

	void close() {
		_window.close();
	}

	void set_title(const std::string& title) {
		_window.setTitle(title);
	}

	bool has_focus() {
		return _window.hasFocus();
	}

	void set_scale(uint32_t scale) {
		_scale = std::clamp(scale, WINDOW_SCALE_MIN, WINDOW_SCALE_MAX);
		_window.setSize(sf::Vector2u(
			_scale * WINDOW_VIEW_WIDTH,
			_scale * WINDOW_VIEW_HEIGHT));
	}

	uint32_t get_scale() {
		return _scale;
	}
}

