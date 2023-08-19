#pragma once
#include <SFML/System/Vector2.hpp>

namespace sf
{
	bool is_zero(const Vector2f& v);
	float len_sqr(const Vector2f& v);
	float len(const Vector2f& v);
	[[nodiscard]] Vector2f normalized(const Vector2f& v);
	[[nodiscard]] Vector2f normalized_safe(const Vector2f& v); // Returns zero vector if v is zero
	void normalize(Vector2f& v);
	void normalize_safe(Vector2f& v); // If v is zero, does nothing
	float dot(const Vector2f& a, const Vector2f& b);
	float cross(const Vector2f& a, const Vector2f& b); // 2D cross product, aka determinant
	float angle_between(const Vector2f& a, const Vector2f& b); // Returns angle in radians
}

