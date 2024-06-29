#pragma once
#ifdef BUILD_IMGUI

struct GLFWwindow;

namespace imgui_backends
{
	void initialize(GLFWwindow* window);
	void shutdown();
	void new_frame();
	void render();
}

#endif // BUILD_IMGUI
