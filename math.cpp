#include "stdafx.h"
#include "math.h"

float smoothstep(float x) {
	return x * x * (3.f - 2.f * x);
}

float smootherstep(float x) {
	return x * x * x * (x * (x * 6.f - 15.f) + 10.f);
}

float lerp(float a, float b, float t) {
	return a + (b - a) * t;
}

float lerp_angle(float a, float b, float t) {
	float angle = fmod(b - a, M_2PI);
	if (angle > M_PI) angle -= M_2PI;
	else if (angle < -M_PI) angle += M_2PI;
	return a + angle * t;
}

bool is_zero(const Vector2f& v) {
	return v.x == 0 && v.y == 0;
}

float length_squared(const Vector2f& v) {
	return v.x * v.x + v.y * v.y;
}

float length(const Vector2f& v) {
	return sqrt(length_squared(v));
}

Vector2f unit_vector(float angle) {
	return Vector2f(cos(angle), sin(angle));
}

Vector2f normalize(const Vector2f& v) {
	if (float len = length(v))
		return v / len;
	return Vector2f(0.f, 0.f);
}

Vector2f abs(const Vector2f& v) {
	return Vector2f(abs(v.x), abs(v.y));
}

Vector2f rotate_90deg(const Vector2f& v) {
	return Vector2f(-v.y, v.x);
}

float dot(const Vector2f& a, const Vector2f& b) {
	return a.x * b.x + a.y * b.y;
}

float det(const Vector2f& a, const Vector2f& b) {
	return a.x * b.y - a.y * b.x;
}

float angle_unsigned(const Vector2f& a, const Vector2f& b) {
	if (float len2 = length_squared(a) * length_squared(b)) {
		return acos(dot(a, b) / sqrt(len2));
	}
	return 0.f;
}

float angle_signed(const Vector2f& a, const Vector2f& b) {
	return atan2(det(a, b), dot(a, b));
}

bool is_clockwise(const Vector2f& a, const Vector2f& b) {
	return det(a, b) > 0; // Since y-axis is down, this is the opposite of the usual definition.
}

Vector2f rotate(const Vector2f& v, float angle) {
	float c = cos(angle);
	float s = sin(angle);
	return Vector2f(v.x * c - v.y * s, v.x * s + v.y * c);
}

Vector2f min(const Vector2f& a, const Vector2f& b) {
	return Vector2f(std::min(a.x, b.x), std::min(a.y, b.y));
}

Vector2f max(const Vector2f& a, const Vector2f& b) {
	return Vector2f(std::max(a.x, b.x), std::max(a.y, b.y));
}

Vector2f lerp(const Vector2f& a, const Vector2f& b, float t) {
	return a + (b - a) * t;
}

Vector2f lerp_polar(const Vector2f& a, const Vector2f& b, float t) {
	float len = lerp(length(a), length(b), t);
	float angle = lerp_angle(atan2(a.y, a.x), atan2(b.y, b.x), t);
	return unit_vector(angle) * len;
}

Vector2f damp(const Vector2f& a, const Vector2f& b, float damping, float dt) {
	damping = std::clamp(damping, 0.f, 1.f);
	dt = std::max(dt, 0.f);
	if (!damping && !dt) return a;
	return lerp(a, b, 1.f - std::pow(damping, dt));
}

Vector2f clamp(const Vector2f& v, const Vector2f& min, const Vector2f& max) {
	return Vector2f(
		std::clamp(v.x, min.x, max.x),
		std::clamp(v.y, min.y, max.y)
	);
}

char get_direction(const Vector2f& v) {
	if (v.x >= +abs(v.y)) return 'r';
	if (v.x <= -abs(v.y)) return 'l';
	if (v.y >= +abs(v.x)) return 'd';
	if (v.y <= -abs(v.x)) return 'u';
	return ' '; // This should never happen.
}

bool is_convex(std::span<const Vector2f> polygon) {
	float first_nonzero_det = 0;
	const size_t vertex_count = polygon.size();
	for (size_t i = 0; i < vertex_count; ++i) {
		size_t i0 = i;
		size_t i1 = (i + 1) % vertex_count;
		size_t i2 = (i + 2) % vertex_count;
		float current_det = det(polygon[i0] - polygon[i1], polygon[i2] - polygon[i1]);
		if (!current_det) continue;
		if (!first_nonzero_det) {
			first_nonzero_det = current_det;
			continue;
		}
		if (current_det * first_nonzero_det < 0) return false;
	}
	return true;
}

std::vector<Vector2f> triangulate(std::span<const Vector2f> polygon) {
	//Ear clipping algorithm: https://www.youtube.com/watch?v=d9tytAQbpXM
	size_t vertex_count = polygon.size();
	assert(vertex_count >= 3);
	if (vertex_count == 3) {
		return { polygon[0], polygon[1], polygon[2] };
	}
	bool is_polygon_clockwise = false;
	{
		size_t i1 = 0; // Will be the rightmost vertex.
		for (size_t i = 1; i < vertex_count; ++i) {
			if (polygon[i].x > polygon[i1].x) {
				i1 = i;
			}
		}
		size_t i0 = (i1 + vertex_count - 1) % vertex_count;
		size_t i2 = (i1 + 1) % vertex_count;
		Vector2f v10 = polygon[i0] - polygon[i1];
		Vector2f v12 = polygon[i2] - polygon[i1];
		is_polygon_clockwise = is_clockwise(v10, v12);
	}
	std::vector<float> angles(vertex_count);
	for (size_t i = 0; i < vertex_count; ++i) {
		size_t i0 = (i + vertex_count - 1) % vertex_count;
		size_t i1 = i;
		size_t i2 = (i + 1) % vertex_count;
		Vector2f v10 = polygon[i0] - polygon[i1];
		Vector2f v12 = polygon[i2] - polygon[i1];
		float angle = angle_unsigned(v10, v12);
		if (is_clockwise(v10, v12) != is_polygon_clockwise) {
			angle = M_2PI - angle;
		}
		angles[i1] = angle;
	}
	std::vector<Vector2f> polygon_copy(polygon.begin(), polygon.end());
	std::vector<Vector2f> triangles;
	while (vertex_count > 3) {
		size_t i2 = 0; // Will be the ear tip, i.e. the vertex with the smallest angle.
		for (size_t i = 1; i < vertex_count; ++i) {
			if (angles[i] < angles[i2]) {
				i2 = i;
			}
		}
		size_t i0 = (i2 + vertex_count - 2) % vertex_count;
		size_t i1 = (i2 + vertex_count - 1) % vertex_count;
		size_t i3 = (i2 + 1) % vertex_count;
		size_t i4 = (i2 + 2) % vertex_count;
		triangles.push_back(polygon_copy[i1]);
		triangles.push_back(polygon_copy[i2]);
		triangles.push_back(polygon_copy[i3]);
		{
			Vector2f v10 = polygon_copy[i0] - polygon_copy[i1];
			Vector2f v13 = polygon_copy[i3] - polygon_copy[i1];
			float angle = angle_unsigned(v10, v13);
			if (is_clockwise(v10, v13) != is_polygon_clockwise) {
				angle = M_2PI - angle;
			}
			angles[i1] = angle;
		}
		{
			Vector2f v31 = polygon_copy[i1] - polygon_copy[i3];
			Vector2f v34 = polygon_copy[i4] - polygon_copy[i3];
			float angle = angle_unsigned(v31, v34);
			if (is_clockwise(v31, v34) != is_polygon_clockwise) {
				angle = M_2PI - angle;
			}
			angles[i3] = angle;
		}
		polygon_copy.erase(polygon_copy.begin() + i2);
		angles.erase(angles.begin() + i2);
		--vertex_count;
	}
	triangles.push_back(polygon_copy[0]);
	triangles.push_back(polygon_copy[1]);
	triangles.push_back(polygon_copy[2]);
	return triangles;
}
