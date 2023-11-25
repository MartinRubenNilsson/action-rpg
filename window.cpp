#include "window.h"
#include "defines.h"

namespace window
{
	sf::RenderWindow* _window = nullptr;
	uint32_t _scale = 5;

	void initialize(sf::RenderWindow& window)
	{
		_window = &window;

		sf::VideoMode video_mode(
			_scale * VIEW_PIXEL_WIDTH,
			_scale * VIEW_PIXEL_HEIGHT);
		std::string title = "Untitled RPG";
		sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
		_window->create(video_mode, title, style);

		sf::View view(sf::FloatRect(
			0, 0, VIEW_PIXEL_WIDTH, VIEW_PIXEL_HEIGHT));
		_window->setView(view);
		_window->setKeyRepeatEnabled(false);
	}

	void close() {
		_window->close();
	}

	void set_title(const std::string& title) {
		_window->setTitle(title);
	}

	bool has_focus() {
		return _window->hasFocus();
	}

	void set_scale(uint32_t scale) {
		_scale = std::clamp(scale, WINDOW_SCALE_MIN, WINDOW_SCALE_MAX);
		_window->setSize(sf::Vector2u(
			_scale * VIEW_PIXEL_WIDTH,
			_scale * VIEW_PIXEL_HEIGHT));
	}

	uint32_t get_scale() {
		return _scale;
	}
}

