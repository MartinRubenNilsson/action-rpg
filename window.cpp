#include "stdafx.h"
#include "window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "window_events.h"
#include "console.h"

namespace window
{
	const Vector2u BASE_SIZE(320, 180); //TODO: remove dependency on sfml

	GLFWwindow* _glfw_window = nullptr;
	std::queue<Event> _event_queue;
	
	void _error_callback(int error, const char* description)
	{
		console::log_error(std::format("GLFW error {}: {}", error, description));
	}

	void _window_close_callback(GLFWwindow* window)
	{
		// GLFW sets the close flag before invoking this callback,
		// so we need to unset it so the window doesn't immediately close.
		glfwSetWindowShouldClose(window, GLFW_FALSE);
		_event_queue.emplace(EventType::WindowClose);
	}

	void _window_size_callback(GLFWwindow* window, int width, int height)
	{
		Event ev{};
		ev.type = EventType::WindowSize;
		ev.size.width = width;
		ev.size.height = height;
		_event_queue.push(ev);
	}

	void _framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		Event ev{};
		ev.type = EventType::FramebufferSize;
		ev.size.width = width;
		ev.size.height = height;
		_event_queue.push(ev);
	}

	void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Event ev{};
		if (action == GLFW_PRESS) {
			ev.type = EventType::KeyPress;
		} else if (action == GLFW_RELEASE) {
			ev.type = EventType::KeyRelease;
		} else if (action == GLFW_REPEAT) {
			ev.type = EventType::KeyRepeat;
		} else {
			return;
		}
		ev.key.code = (Key)key;
		ev.key.scancode = scancode;
		//ev.key.mods = mods;
		_event_queue.push(ev);
	}

	void _cursor_pos_callback(GLFWwindow* window, double x, double y)
	{
		Event ev{};
		ev.type = EventType::MouseMove;
		ev.mouse_move.x = x;
		ev.mouse_move.y = y;
		_event_queue.push(ev);
	}

	bool initialize()
	{
		glfwSetErrorCallback(_error_callback);
		if (!glfwInit()) return false;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
#ifdef OPENGL_PROFILE_COMPATIBILITY
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#else
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
		const unsigned int scale = 4;
		_glfw_window = glfwCreateWindow(BASE_SIZE.x * scale, BASE_SIZE.y * scale, "Action RPG", nullptr, nullptr);
		if (!_glfw_window) return false;

		glfwMakeContextCurrent(_glfw_window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

		glfwSetWindowCloseCallback(_glfw_window, _window_close_callback);
		glfwSetKeyCallback(_glfw_window, _key_callback);
		glfwSetWindowSizeCallback(_glfw_window, _window_size_callback);
		glfwSetFramebufferSizeCallback(_glfw_window, _framebuffer_size_callback);
		glfwSetCursorPosCallback(_glfw_window, _cursor_pos_callback);

		glfwSwapInterval(0); // Disable vsync
		set_icon_from_file("assets/window/swordsman.png");

		// Spoof resize events to ensure that other systems are aware of the window/framebuffer size.
		int width, height;
		glfwGetWindowSize(_glfw_window, &width, &height);
		_window_size_callback(_glfw_window, width, height);
		_framebuffer_size_callback(_glfw_window, width, height);

		return true;
	}

	void shutdown()
	{
		glfwDestroyWindow(_glfw_window);
		glfwTerminate();
	}

	GLFWwindow* get_glfw_window()
	{
		return _glfw_window;
	}

	double get_elapsed_time()
	{
		return glfwGetTime();
	}

	bool should_close()
	{
		return glfwWindowShouldClose(_glfw_window);
	}

	void set_should_close(bool should_close)
	{
		glfwSetWindowShouldClose(_glfw_window, should_close);
	}

	void poll_events()
	{
		glfwPollEvents();
	}

	bool get_next_event(Event& ev)
	{
		if (_event_queue.empty()) return false;
		ev = _event_queue.front();
		_event_queue.pop();
		return true;
	}

	void swap_buffers()
	{
		glfwSwapBuffers(_glfw_window);
	}

	bool has_focus()
	{
		return glfwGetWindowAttrib(_glfw_window, GLFW_FOCUSED);
	}

	void set_size(int width, int height)
	{
		glfwSetWindowSize(_glfw_window, width, height);
	}

	void get_size(int& width, int& height)
	{
		glfwGetWindowSize(_glfw_window, &width, &height);
	}

	void get_framebuffer_size(int& width, int& height)
	{
		glfwGetFramebufferSize(_glfw_window, &width, &height);
	}

	void set_title(const std::string& title)
	{
		glfwSetWindowTitle(_glfw_window, title.c_str());
	}

	void set_icon_from_memory(int width, int height, unsigned char* pixels)
	{
		GLFWimage image{};
		image.width = width;
		image.height = height;
		image.pixels = pixels;
		glfwSetWindowIcon(_glfw_window, 1, &image);
	}

	void set_icon_from_file(const std::string& path)
	{
		int width, height, channels;
		unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
		if (!pixels) {
			console::log_error("Failed to load icon: " + path);
			return;
		}
		set_icon_from_memory(width, height, pixels);
		stbi_image_free(pixels);
	}

	void set_cursor_visible(bool visible)
	{
		glfwSetInputMode(_glfw_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
	}

	bool get_cursor_visible()
	{
		return glfwGetInputMode(_glfw_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
	}

	void set_cursor_pos(double x, double y)
	{
		glfwSetCursorPos(_glfw_window, x, y);
	}

	void get_cursor_pos(double& x, double& y)
	{
		glfwGetCursorPos(_glfw_window, &x, &y);
	}

	void set_clipboard_string(const std::string& string)
	{
		glfwSetClipboardString(_glfw_window, string.c_str());
	}

	std::string get_clipboard_string()
	{
		const char* string = glfwGetClipboardString(_glfw_window);
		return string ? string : "";
	}

#if 0
	void set_state(const State& state)
	{
		// We need to recreate the window to change the fullscreen mode.
		bool recreate = (!_window->isOpen() || state.fullscreen != _state.fullscreen);
		if (recreate) {
			Vector2u size = state.scale * BASE_SIZE;
			sf::VideoMode mode = state.fullscreen ?
				sf::VideoMode::getFullscreenModes().at(0) :
				sf::VideoMode(size.x, size.y);
			sf::Uint32 style = state.fullscreen ? 
				sf::Style::Fullscreen : (sf::Style::Titlebar | sf::Style::WindowClose);
			sf::ContextSettings settings{};
			settings.majorVersion = 4;
			settings.minorVersion = 3;
#ifdef _DEBUG
			settings.attributeFlags |= sf::ContextSettings::Attribute::Debug;
#endif
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
			_window->setView(sf::View(Vector2f(size) / 2.f, Vector2f(size)));
		}
		if (recreate || state.vsync != _state.vsync)
			_window->setVerticalSyncEnabled(state.vsync);
		if (recreate || state.cursor_visible != _state.cursor_visible)
			_window->setMouseCursorVisible(state.cursor_visible);
		_state = state;
	}
#endif
}

