#include "window.h"
#include "defines.h"

namespace window
{
	sf::RenderWindow _window;

	sf::RenderWindow& create()
	{
		sf::VideoMode video_mode(VIEW_WIDTH * 5, VIEW_HEIGHT * 5);
		std::string title = "Untitled RPG";
		sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
		_window.create(video_mode, title, style);

		sf::View view(sf::FloatRect(0, 0, VIEW_WIDTH, VIEW_HEIGHT));
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
}

