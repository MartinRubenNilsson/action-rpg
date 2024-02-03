#include "window.h"

namespace window
{
	const size_t _SYSTEM_CURSOR_COUNT = (size_t)sf::Cursor::Type::NotAllowed;

	sf::RenderWindow* _window = nullptr;
	std::array<sf::Cursor, _SYSTEM_CURSOR_COUNT> _system_cursors;
	Desc _desc;
	std::vector<sf::Event> _custom_event_queue;

	const Desc& get_desc() {
		return _desc;
	}

	void initialize(sf::RenderWindow& window)
	{
		_window = &window;
		for (size_t i = 0; i < _SYSTEM_CURSOR_COUNT; ++i)
			_system_cursors[i].loadFromSystem((sf::Cursor::Type)i);
	}

	void create_or_update(const Desc& desc)
	{
		// We need to recreate the window to change the fullscreen mode.
		bool recreate = (!_window->isOpen() || desc.fullscreen != _desc.fullscreen);
		if (recreate) {
			sf::VideoMode mode = desc.fullscreen ?
				sf::VideoMode::getFullscreenModes().at(0) :
				sf::VideoMode(desc.scale * VIEW_SIZE.x, desc.scale * VIEW_SIZE.y);
			sf::Uint32 style = desc.fullscreen ? 
				sf::Style::Fullscreen : (sf::Style::Titlebar | sf::Style::Close);
			_window->create(mode, desc.title, style);
			_window->setKeyRepeatEnabled(false);
			// Spoof a resize event to ensure that other systems are aware of the new window size.
			sf::Event ev{};
			ev.type = sf::Event::Resized;
			ev.size.width = mode.width;
			ev.size.height = mode.height;
			_custom_event_queue.push_back(ev);
		}
		if (desc.title != _desc.title)
			_window->setTitle(desc.title);
		if (recreate || desc.icon_filename != _desc.icon_filename) {
			sf::Image icon;
			if (icon.loadFromFile(desc.icon_filename))
				_window->setIcon(
					icon.getSize().x,
					icon.getSize().y,
					icon.getPixelsPtr());
		}
		if (!recreate && desc.scale != _desc.scale) {
			sf::Vector2u new_size(desc.scale * VIEW_SIZE.x, desc.scale * VIEW_SIZE.y);
			_window->setSize(new_size);
			_window->setView(sf::View(sf::FloatRect(0.f, 0.f, (float)new_size.x, (float)new_size.y)));
		}
		if (recreate || desc.cursor_visible != _desc.cursor_visible)
			_window->setMouseCursorVisible(desc.cursor_visible);
		if (recreate || desc.vsync != _desc.vsync)
			_window->setVerticalSyncEnabled(desc.vsync);
		_desc = desc;
	}

	bool poll_event(sf::Event& ev)
	{
		if (_custom_event_queue.empty())
			return _window->pollEvent(ev);
		ev = _custom_event_queue.back();
		_custom_event_queue.pop_back();
		return true;
	}

	void set_cursor(sf::Cursor::Type type) {
		_window->setMouseCursor(_system_cursors[(size_t)type]);
	}

	bool has_focus() {
		return _window->hasFocus();
	}
}

