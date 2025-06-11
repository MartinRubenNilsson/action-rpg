#include "stdafx.h"
#ifdef _DEBUG
#include "kdtree_test.h"
#include "kdtree.h"
#include "random.h"
#include "shapes.h"

namespace kdtree_test {
	int _size = 1000;
	std::vector<Vector2f> _kdtree;
	double _cursor_x = 0;
	double _cursor_y = 0;

	void _generate_points(int framebuffer_width, int framebuffer_height) {
		_kdtree.resize(_size);
		for (Vector2f& point : _kdtree) {
			point.x = random::range_f(0.f, (float)framebuffer_width);
			point.y = random::range_f(0.f, (float)framebuffer_height);
		}
	}

	void show_imgui_window(int framebuffer_width, int framebuffer_height) {
#ifdef _DEBUG_IMGUI
		ImGui::Begin("k-d tree");
		if (ImGui::InputInt("Size", &_size)) {
			_size = std::max(_size, 0);
		}
		if (ImGui::Button("Build by sorting")) {
			_generate_points(framebuffer_width, framebuffer_height);
			kdtree::build_by_sorting(_kdtree);
		}
		ImGui::End();
#endif
	}

	void set_cursor_pos(double x, double y) {
		_cursor_x = x;
		_cursor_y = y;
	}

	void add_debug_shapes_to_render_queue() {
		const Vector2f cursor_position = { (float)_cursor_x, (float)_cursor_y };
		size_t nearest_index = SIZE_MAX;
		float nearest_distance_sq = FLT_MAX;
		kdtree::query_nearest(_kdtree, cursor_position, nearest_index, nearest_distance_sq);
		for (size_t index = 0; index < _kdtree.size(); ++index) {
			const bool is_nearest = (index == nearest_index);
			shapes::add_circle(_kdtree[index], is_nearest ? 2.f : 1.f, is_nearest ? colors::GREEN : colors::WHITE);
			if (is_nearest) {
				shapes::add_circle(cursor_position, sqrt(nearest_distance_sq), colors::GREEN);
			}
		}

	}
}

#endif
