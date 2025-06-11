#pragma once
#ifdef _DEBUG

namespace kdtree_test {
	void show_imgui_window(int framebuffer_width, int framebuffer_height);
	void set_cursor_pos(double x, double y);
	void add_debug_shapes_to_render_queue();
}

#endif