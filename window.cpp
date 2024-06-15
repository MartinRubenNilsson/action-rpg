#include "stdafx.h"
#include "window.h"

namespace window
{
	const sf::Vector2u BASE_SIZE(320, 180);
	const sf::View BASE_VIEW(sf::FloatRect(0.f, 0.f, (float)BASE_SIZE.x, (float)BASE_SIZE.y));
	const size_t _SYSTEM_CURSOR_COUNT = (size_t)sf::Cursor::Type::NotAllowed;

	sf::RenderWindow* _window = nullptr;
	std::array<sf::Cursor, _SYSTEM_CURSOR_COUNT> _system_cursors;
	State _state;
	std::vector<sf::Event> _custom_event_queue;

	const State& get_state() {
		return _state;
	}

	void initialize(sf::RenderWindow& window)
	{
		_window = &window;
		for (size_t i = 0; i < _SYSTEM_CURSOR_COUNT; ++i)
			_system_cursors[i].loadFromSystem((sf::Cursor::Type)i);
	}

	void set_state(const State& state)
	{
		// We need to recreate the window to change the fullscreen mode.
		bool recreate = (!_window->isOpen() || state.fullscreen != _state.fullscreen);
		if (recreate) {
			sf::Vector2u size = state.scale * BASE_SIZE;
			sf::VideoMode mode = state.fullscreen ?
				sf::VideoMode::getFullscreenModes().at(0) :
				sf::VideoMode(size.x, size.y);
			sf::Uint32 style = state.fullscreen ? 
				sf::Style::Fullscreen : (sf::Style::Titlebar | sf::Style::Close);
			sf::ContextSettings settings{};
			settings.majorVersion = 3;
			settings.minorVersion = 3;
//#ifdef _DEBUG
//			settings.attributeFlags |= sf::ContextSettings::Attribute::Debug;
//#endif
			_window->create(mode, state.title, style, settings);
			_window->setKeyRepeatEnabled(false);
			// Spoof a resize event to ensure that other systems are aware of the new window size.
			sf::Event ev{};
			ev.type = sf::Event::Resized;
			ev.size.width = mode.width;
			ev.size.height = mode.height;
			_custom_event_queue.push_back(ev);
		}
		if (state.title != _state.title)
			_window->setTitle(state.title);
		if (recreate || state.icon != _state.icon) {
			sf::Image icon;
			if (icon.loadFromFile(state.icon))
				_window->setIcon(
					icon.getSize().x,
					icon.getSize().y,
					icon.getPixelsPtr());
		}
		if (!recreate && state.scale != _state.scale) {
			sf::Vector2 size = state.scale * BASE_SIZE;
			_window->setSize(size);
			_window->setView(sf::View(sf::Vector2f(size) / 2.f, sf::Vector2f(size)));
		}
		if (recreate || state.vsync != _state.vsync)
			_window->setVerticalSyncEnabled(state.vsync);
		if (recreate || state.cursor_visible != _state.cursor_visible)
			_window->setMouseCursorVisible(state.cursor_visible);
		_state = state;
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

