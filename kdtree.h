#pragma once
#include "vector2.h"
#include <span>

namespace kdtree {
	// Complexity: O(n log^2 n)
	void build_by_sorting(std::span<Vector2f> points);
}