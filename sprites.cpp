#include "stdafx.h"
#include "sprites.h"

namespace sprites
{
	bool operator<(const Sprite& left, const Sprite& right)
	{
		if (left.sorting_layer != right.sorting_layer)
			return left.sorting_layer < right.sorting_layer;
		if (left.sorting_position.y != right.sorting_position.y)
			return left.sorting_position.y < right.sorting_position.y;
		if (left.sorting_position.x != right.sorting_position.x)
			return left.sorting_position.x < right.sorting_position.x;
		if (left.texture != right.texture)
			return left.texture < right.texture;
		if (left.shader != right.shader)
			return left.shader < right.shader;
		return false;
	}
}
