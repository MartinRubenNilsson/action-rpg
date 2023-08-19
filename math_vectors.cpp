#include "math_vectors.h"

namespace sf
{
	bool is_zero(const Vector2f& v) {
		return v.x == 0 && v.y == 0;
	}

	float len_sqr(const Vector2f& v) {
		return v.x * v.x + v.y * v.y;
	}

	float len(const Vector2f& v) {
		return std::sqrt(len_sqr(v));
	}

	Vector2f normalized(const Vector2f& v) {
		return v / len(v);
	}

	Vector2f normalized_safe(const Vector2f& v) {
		if (float l = len(v)) return v / l;
		return Vector2f(0, 0);
	}

	void normalize(Vector2f& v) {
		v /= len(v);
	}

	void normalize_safe(Vector2f& v) {
		if (float l = len(v)) v /= l;
	}

	float dot(const Vector2f& a, const Vector2f& b) {
		return a.x * b.x + a.y * b.y;
	}

	float cross(const Vector2f& a, const Vector2f& b) {
		return a.x * b.y - a.y * b.x;
	}

	float angle_between(const Vector2f& a, const Vector2f& b) {
		return std::acos(dot(a, b) / (len(a) * len(b))); // TODO: Check for division by zero
	}
}