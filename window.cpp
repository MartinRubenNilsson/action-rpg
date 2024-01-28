#include "window.h"

namespace window
{
	const sf::Vector2u VIEW_SIZE(320u, 180u);
	const uint32_t MIN_SCALE = 3u;
	const uint32_t MAX_SCALE = 7u;
	const size_t _SYSTEM_CURSOR_COUNT = (size_t)sf::Cursor::Type::NotAllowed;

	sf::RenderWindow* _window = nullptr;
	uint32_t _scale = 5;
	std::array<sf::Cursor, _SYSTEM_CURSOR_COUNT> _system_cursors;

	void initialize(sf::RenderWindow& window)
	{
		_window = &window;
		sf::VideoMode video_mode(_scale * VIEW_SIZE.x, _scale * VIEW_SIZE.y);
		std::string title = "Action RPG";
		sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
		_window->create(video_mode, title, style);
		sf::View view(sf::FloatRect(0.f, 0.f, (float)VIEW_SIZE.x, (float)VIEW_SIZE.y));
		_window->setView(view);
		_window->setKeyRepeatEnabled(false);
		set_icon("assets/window/swordsman.png");
		for (size_t i = 0; i < _SYSTEM_CURSOR_COUNT; ++i)
			_system_cursors[i].loadFromSystem((sf::Cursor::Type)i);
	}

	void close() {
		_window->close();
	}

	void set_title(const std::string& title) {
		_window->setTitle(title);
	}

	bool set_icon(const std::string& filename)
	{
		sf::Image icon;
		if (!icon.loadFromFile(filename)) return false;
		_window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
		return true;
	}

	void set_cursor_visible(bool visible) {
		_window->setMouseCursorVisible(visible);
	}

	void set_cursor(sf::Cursor::Type type) {
		_window->setMouseCursor(_system_cursors[(size_t)type]);
	}

	bool has_focus() {
		return _window->hasFocus();
	}

	void set_scale(uint32_t scale)
	{
		_scale = std::clamp(scale, MIN_SCALE, MAX_SCALE);
		_window->setSize(_scale * VIEW_SIZE);
	}

	uint32_t get_scale() {
		return _scale;
	}
}

