#include "window.h"

namespace window
{
	const std::string _TITLE = "Action RPG";
	const sf::Vector2u VIEW_SIZE(320u, 180u);
	const uint32_t MIN_SCALE = 3u;
	const uint32_t MAX_SCALE = 7u;
	const size_t _SYSTEM_CURSOR_COUNT = (size_t)sf::Cursor::Type::NotAllowed;

	std::unique_ptr<sf::RenderWindow> _window;
	std::vector<sf::Event> _custom_event_queue;
	uint32_t _scale = 5;
	std::array<sf::Cursor, _SYSTEM_CURSOR_COUNT> _system_cursors;
	// sf::Window doesn't have getters for these, so we'll need to track them ourselves. Not ideal, but whatever.
	bool _is_cursor_visible = true;
	bool _is_fullscreen = false;

	sf::VideoMode _get_windowed_mode() {
		return sf::VideoMode(_scale * VIEW_SIZE.x, _scale * VIEW_SIZE.y);
	}

	sf::VideoMode _get_fullscreen_mode() {
		return sf::VideoMode::getFullscreenModes().at(0);
	}

	sf::VideoMode _get_current_mode() {
		return _is_fullscreen ? _get_fullscreen_mode() : _get_windowed_mode();
	}

	sf::Uint32 _get_current_style() {
		return _is_fullscreen ? sf::Style::Fullscreen : (sf::Style::Titlebar | sf::Style::Close);
	}

	void _recreate_window()
	{
		sf::VideoMode mode = _get_current_mode();
		_window->create(mode, _TITLE, _get_current_style());
		_window->setView(sf::View(sf::FloatRect(0.f, 0.f, (float)VIEW_SIZE.x, (float)VIEW_SIZE.y)));
		_window->setKeyRepeatEnabled(false);
		set_icon("assets/window/swordsman.png");

		// Inject a resize event to ensure that the game is aware of the new window size.
		sf::Event resize_event{};
		resize_event.type = sf::Event::Resized;
		resize_event.size.width = mode.width;
		resize_event.size.height = mode.height;
		_custom_event_queue.push_back(resize_event);
	}

	sf::RenderWindow& create()
	{
		_window = std::make_unique<sf::RenderWindow>();
		_recreate_window();
		for (size_t i = 0; i < _SYSTEM_CURSOR_COUNT; ++i)
			_system_cursors[i].loadFromSystem((sf::Cursor::Type)i);
		return *_window;
	}

	void destroy() {
		_window.reset();
	}

	void close() {
		_window->close();
	}

	bool poll_event(sf::Event& ev)
	{
		if (_custom_event_queue.empty())
			return _window->pollEvent(ev);
		ev = _custom_event_queue.back();
		_custom_event_queue.pop_back();
		return true;
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

	void set_cursor_visible(bool visible)
	{
		_window->setMouseCursorVisible(visible);
		_is_cursor_visible = visible;
	}

	bool is_cursor_visible(){
		return _is_cursor_visible;
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

	void set_fullscreen(bool fullscreen)
	{
		if (fullscreen == _is_fullscreen) return;
		_is_fullscreen = fullscreen;
		_recreate_window();
	}

	bool is_fullscreen() {
		return _is_fullscreen;
	}
}

