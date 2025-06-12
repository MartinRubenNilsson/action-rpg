#include "stdafx.h"
#include "delaunay.h"

namespace delaunay {
	float _determinant(
		float a, float b, float c,
		float d, float e, float f,
		float g, float h, float i
	) {
		// Sarrus' determinant formula for a 3x3 matrix
		return a * e * i + b * f * g + c * d * h - c * e * g - b * d * i - a * f * h;
	}

	// Check if point d is inside the circumcircle of triangle abc.
	// Returns true if point d is inside the circumcircle and a, b, c are oriented counter-clockwise,
	// or if point d is outside the circumcircle and a, b, c are oriented clockwise, and returns false otherwise.
	bool _in_circle(const Vector2f& a, const Vector2f& b, const Vector2f& c, const Vector2f& d) {
		const Vector2f da = a - d;
		const Vector2f db = b - d;
		const Vector2f dc = c - d;
		const float det = _determinant(
			da.x, da.y, length_squared(da),
			db.x, db.y, length_squared(db),
			dc.x, dc.y, length_squared(dc)
		);
		return det > 0.f;
	}

	void build_bowyer_watson(std::span<const Vector2f> points, std::vector<unsigned int>& indices) {
		//TODO
	}
}