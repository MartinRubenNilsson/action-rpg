#pragma once
#include "vector2.h"
#include <span>

namespace kdtree {
	// Complexity: O(n log^2 n)
	void build_by_sorting(std::span<Vector2f> kdtree);
	// Complexity: O(log n)
	void query_nearest(std::span<const Vector2f> kdtree, const Vector2f& point, size_t& nearest_index, float& nearest_distance_sq);
}