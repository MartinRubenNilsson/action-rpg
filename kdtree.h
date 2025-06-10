#pragma once
#include "vector2.h"
#include <span>

namespace kdtree {
	void build(std::span<Vector2f> points);
}