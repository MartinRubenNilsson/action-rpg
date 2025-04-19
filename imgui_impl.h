#pragma once
#ifdef _DEBUG_IMGUI

namespace imgui_impl {
	void initialize();
	void shutdown();
	void new_frame();
	void render();
}

#endif // _DEBUG_IMGUI