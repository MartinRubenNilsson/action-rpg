#include "stdafx.h"
#include "kdtree.h"

namespace kdtree {
	bool _compare_x(const Vector2f& a, const Vector2f& b) {
		return a.x < b.x;
	}

	bool _compare_y(const Vector2f& a, const Vector2f& b) {
		return a.y < b.y;
	}

	void _build_by_sorting_recursively(std::span<Vector2f> points, bool split_x) {
		const size_t size = points.size();
		if (size <= 1) return;
		std::sort(points.begin(), points.end(), split_x ? _compare_x : _compare_y);
		const size_t left_size = size / 2;
		_build_by_sorting_recursively(points.first(left_size), !split_x);
		const size_t right_size = size - left_size - 1;
		_build_by_sorting_recursively(points.last(right_size), !split_x);
	}

	void build_by_sorting(std::span<Vector2f> points) {
		_build_by_sorting_recursively(points, true);
	}
}