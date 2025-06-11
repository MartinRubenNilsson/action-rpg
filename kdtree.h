#pragma once
#include "vector2.h"
#include <span>

namespace kdtree {
	// Builds a k-d tree from a set of points. Complexity: O(n log^2 n)
	void build_by_sorting(std::span<Vector2f> kdtree);

	// Builds a k-d tree from a set of points. Complexity: O(n log n)
	void build_by_partitioning(std::span<Vector2f> kdtree);

	// Queries the nearest point to a given point in the k-d tree. Sets 'nearest_index' to the index of the nearest point
	// in the k-d tree, and 'nearest_distance_sq' to the squared distance to that point. You can set 'nearest_distance_sq'
	// < FLT_MAX to restrict the search to points within a certain distance. Sets 'nearest_index' to SIZE_MAX if no point
	// is found within the distance, or if the tree is empty. Complexity: O(log n)
	void query_nearest(std::span<const Vector2f> kdtree, const Vector2f& point, size_t& nearest_index, float& nearest_distance_sq);
}