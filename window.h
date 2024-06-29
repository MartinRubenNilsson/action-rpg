#pragma once

struct GLFWwindow;

namespace window
{
	struct Event;
	extern const Vector2u BASE_SIZE;

#if 0
	struct State
	{
		std::string title = "Action RPG";
		std::string icon = "assets/window/swordsman.png"; // Path to icon file.
		uint32_t scale = 5; // Only used if fullscreen is false.
		bool fullscreen = false;
		bool vsync = false;
		bool cursor_visible = true;
	};
#endif

	bool initialize();
	void shutdown();

	GLFWwindow* get_glfw_window();
	double get_elapsed_time(); // Time since the window was created, in seconds.

	bool should_close();
	void set_should_close(bool should_close);

	// Processes all pending events and populates the event queue. Call at the start of each frame.
	void poll_events(); 
	// Get the next event in the event queue. Returns false if there are no more events.
	bool get_next_event(Event& ev);
	// Swaps the front and back buffers. Call at the end of each frame.
	void swap_buffers(); 

	bool has_focus();
	void set_size(int width, int height);
	void get_size(int& width, int& height);
	void get_framebuffer_size(int& width, int& height);
	void set_title(const std::string& title);
	void set_icon_from_memory(int width, int height, unsigned char* pixels);
	void set_icon_from_file(const std::string& path); // uses stb_image internally

	void set_cursor_visible(bool visible);
	bool get_cursor_visible();
	void set_cursor_pos(double x, double y);
	void get_cursor_pos(double& x, double& y);

	void set_clipboard_string(const std::string& string);
	std::string get_clipboard_string();
}
