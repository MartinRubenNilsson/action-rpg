#pragma once

// Clay (short for C Layout) is the 2D UI layout library we use in this project.
// GitHub link: https://github.com/nicbarker/clay

namespace ui {
	bool initialize_clay();
	void shutdown_clay();

	void set_clay_layout_dimensions(float width, float height);
	void set_clay_pointer_state(float x, float y, bool is_down);
	void update_clay_scroll_containers(float scroll_delta_x, float dt);

	void begin_clay_layout();
	void _test_clay();
	void end_clay_layout();
	void render_clay_layout();
}