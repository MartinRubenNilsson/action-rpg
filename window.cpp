#include "stdafx.h"
#include "window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "window_events.h"
#include "console.h"

namespace window
{
	GLFWwindow* _glfw_window = nullptr;
	GLFWcursor* _glfw_cursors[(int)CursorShape::Count] = { nullptr };
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

	int _translate_modifier_key_flags_from_glfw(int glfw_modifier_key_flags) {
		int modifier_key_flags = 0;
		if (glfw_modifier_key_flags & GLFW_MOD_SHIFT)     modifier_key_flags |= MODIFIER_KEY_SHIFT;
		if (glfw_modifier_key_flags & GLFW_MOD_CONTROL)   modifier_key_flags |= MODIFIER_KEY_CONTROL;
		if (glfw_modifier_key_flags & GLFW_MOD_ALT)       modifier_key_flags |= MODIFIER_KEY_ALT;
		if (glfw_modifier_key_flags & GLFW_MOD_SUPER)     modifier_key_flags |= MODIFIER_KEY_SUPER;
		if (glfw_modifier_key_flags & GLFW_MOD_CAPS_LOCK) modifier_key_flags |= MODIFIER_KEY_CAPS_LOCK;
		if (glfw_modifier_key_flags & GLFW_MOD_NUM_LOCK)  modifier_key_flags |= MODIFIER_KEY_NUM_LOCK;
		return modifier_key_flags;
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
		ev.key.modifier_key_flags = _translate_modifier_key_flags_from_glfw(mods);
		_event_queue.push(ev);
	}

	void _mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		Event ev{};
		if (action == GLFW_PRESS) {
			ev.type = EventType::MouseButtonPress;
		} else if (action == GLFW_RELEASE) {
			ev.type = EventType::MouseButtonRelease;
		} else {
			return;
		}
		ev.mouse_button.button = (MouseButton)button;
		ev.mouse_button.modifier_key_flags = _translate_modifier_key_flags_from_glfw(mods);
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
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef DEBUG_GRAPHICS
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hide the window until we're ready to show it.
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		_glfw_window = glfwCreateWindow(WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT, "Action RPG", nullptr, nullptr);
		if (!_glfw_window) return false;

		glfwMakeContextCurrent(_glfw_window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

		glfwSetWindowCloseCallback(_glfw_window, _window_close_callback);
		glfwSetKeyCallback(_glfw_window, _key_callback);
		glfwSetWindowSizeCallback(_glfw_window, _window_size_callback);
		glfwSetFramebufferSizeCallback(_glfw_window, _framebuffer_size_callback);
		glfwSetMouseButtonCallback(_glfw_window, _mouse_button_callback);
		glfwSetCursorPosCallback(_glfw_window, _cursor_pos_callback);

		// Spoof resize events to ensure that other systems are aware of the window/framebuffer size.
		{
			int width, height;
			glfwGetWindowSize(_glfw_window, &width, &height);
			_window_size_callback(_glfw_window, width, height);
			glfwGetFramebufferSize(_glfw_window, &width, &height);
			_framebuffer_size_callback(_glfw_window, width, height);
		}

		// CREATE STANDARD CURSORS

		_glfw_cursors[(int)CursorShape::Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		_glfw_cursors[(int)CursorShape::IBeam] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		_glfw_cursors[(int)CursorShape::Crosshair] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
		_glfw_cursors[(int)CursorShape::Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		_glfw_cursors[(int)CursorShape::HResize] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		_glfw_cursors[(int)CursorShape::VResize] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);

		// LOAD AND CREATE CUSTOM CURSORS

		int width, height, channels;
		unsigned char* pixels = stbi_load("assets/textures/cursors/cursor32x32.png", &width, &height, &channels, 4);
		constexpr int CURSOR_SIZE = 32;
		constexpr int CURSOR_SIZE_BYTES = CURSOR_SIZE * CURSOR_SIZE * 4;
		if (width == CURSOR_SIZE && height == CURSOR_SIZE * 10 && pixels) {
			GLFWimage image{};
			image.height = CURSOR_SIZE;
			image.width = CURSOR_SIZE;
			image.pixels = pixels;
			_glfw_cursors[(int)CursorShape::HandPoint] = glfwCreateCursor(&image, 0, 0);
			image.pixels += CURSOR_SIZE_BYTES;
			_glfw_cursors[(int)CursorShape::HandPointUp] = glfwCreateCursor(&image, 0, 0);
			image.pixels += CURSOR_SIZE_BYTES;
			_glfw_cursors[(int)CursorShape::HandGrab] = glfwCreateCursor(&image, 0, 0);
			image.pixels += CURSOR_SIZE_BYTES;
			_glfw_cursors[(int)CursorShape::Quill] = glfwCreateCursor(&image, 0, 0);
		}
		stbi_image_free(pixels);

		// FINAL SETUP

		set_swap_interval(0); // Disable vsync
		set_cursor_shape(CursorShape::HandPoint);
		set_icon_from_file("assets/window/swordsman.png");

		return true;
	}

	void shutdown()
	{
		for (GLFWcursor* cursor : _glfw_cursors) {
			glfwDestroyCursor(cursor);
		}
		memset(_glfw_cursors, 0, sizeof(_glfw_cursors));
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

	void set_visible(bool visible)
	{
		if (visible) {
			glfwShowWindow(_glfw_window);
		} else {
			glfwHideWindow(_glfw_window);
		}
	}

	bool get_visible()
	{
		return glfwGetWindowAttrib(_glfw_window, GLFW_VISIBLE);
	}

	void set_fullscreen(bool fullscreen)
	{
		if (fullscreen == get_fullscreen()) return;
		static int last_windowed_xpos = 0;
		static int last_windowed_ypos = 0;
		static int last_windowed_width = 0;
		static int last_windowed_height = 0;
		if (fullscreen) {
			GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
			if (!primary_monitor) return;
			const GLFWvidmode* video_mode = glfwGetVideoMode(primary_monitor);
			if (!video_mode) return;
			glfwGetWindowPos(_glfw_window, &last_windowed_xpos, &last_windowed_ypos);
			glfwGetWindowSize(_glfw_window, &last_windowed_width, &last_windowed_height);
			glfwSetWindowMonitor(_glfw_window, primary_monitor,
				0, 0, video_mode->width, video_mode->height, video_mode->refreshRate);
		} else {
			glfwSetWindowMonitor(_glfw_window, nullptr,
				last_windowed_xpos, last_windowed_ypos,
				last_windowed_width, last_windowed_height, GLFW_DONT_CARE);
		}
	}

	bool get_fullscreen()
	{
		return glfwGetWindowMonitor(_glfw_window) != nullptr;
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

	void set_swap_interval(int interval)
	{
		static int last_interval = 0;
		if (interval == last_interval) return;
		last_interval = interval;
		glfwSwapInterval(interval);
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

	void set_cursor_shape(CursorShape shape)
	{
		glfwSetCursor(_glfw_window, _glfw_cursors[(int)shape]);
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
}

