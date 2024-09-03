#pragma once
#ifdef _DEBUG_IMGUI

namespace imgui_backends
{
	void initialize();
	void shutdown();
	void new_frame();
	void render();
}

#endif // DEBUG_IMGUI
