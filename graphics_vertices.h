#pragma once
#include "color.h"
#include "vector2.h"

namespace graphics {
	struct Vertex {
		Vector2f position;
		Color color;
		Vector2f tex_coord;
	};
}