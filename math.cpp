#include "math.h"

float smoothstep(float x) {
	return x * x * (3.f - 2.f * x);
}

float smootherstep(float x) {
	return x * x * x * (x * (x * 6.f - 15.f) + 10.f);
}

sf::Vector2f operator*(const sf::Vector2f& left, const sf::Vector2f& right) {
	return sf::Vector2f(left.x * right.x, left.y * right.y);
}

sf::Vector2f operator/(const sf::Vector2f& left, const sf::Vector2f& right) {
	return sf::Vector2f(left.x / right.x, left.y / right.y);
}

sf::Vector2f& operator*=(sf::Vector2f& left, const sf::Vector2f& right)
{
	left.x *= right.x;
	left.y *= right.y;
	return left;
}

sf::Vector2f& operator/=(sf::Vector2f& left, const sf::Vector2f& right)
{
	left.x /= right.x;
	left.y /= right.y;
	return left;
}

bool is_zero(const sf::Vector2f& v) {
	return v.x == 0 && v.y == 0;
}

float length_squared(const sf::Vector2f& v) {
	return v.x * v.x + v.y * v.y;
}

float length(const sf::Vector2f& v) {
	return std::sqrt(length_squared(v));
}

sf::Vector2f normalize(const sf::Vector2f& v) {
	if (float denom = length(v))
		return v / denom;
	return sf::Vector2f(0, 0);
}

float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
	return a.x * b.x + a.y * b.y;
}

float det(const sf::Vector2f& a, const sf::Vector2f& b) {
	return a.x * b.y - a.y * b.x;
}

float angle_unsigned(const sf::Vector2f& a, const sf::Vector2f& b) {
	if (float denom = length(a) * length(b))
		return std::acos(dot(a, b) / denom);
	return 0.f;
}

float angle_signed(const sf::Vector2f& a, const sf::Vector2f& b) {
	return std::atan2(det(a, b), dot(a, b));
}

bool is_clockwise(const sf::Vector2f& a, const sf::Vector2f& b) {
	return det(a, b) > 0; // Since y-axis is down, this is the opposite of the usual definition.
}

sf::Vector2f rotate(const sf::Vector2f& v, float angle)
{
	float c = std::cos(angle);
	float s = std::sin(angle);
	return sf::Vector2f(v.x * c - v.y * s, v.x * s + v.y * c);
}

sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t) {
	return a + (b - a) * t;
}

sf::Vector2f damp(const sf::Vector2f& a, const sf::Vector2f& b, float damping, float dt)
{
	damping = std::clamp(damping, 0.f, 1.f);
	dt = std::max(dt, 0.f);
	if (!damping && !dt) return a;
	return lerp(a, b, 1.f - std::pow(damping, dt));
}

sf::Vector2f clamp(const sf::Vector2f& v, const sf::Vector2f& min, const sf::Vector2f& max)
{
	return sf::Vector2f(
		std::clamp(v.x, min.x, max.x),
		std::clamp(v.y, min.y, max.y)
	);
}

char get_direction(const sf::Vector2f& v)
{
	if (v.x >= +abs(v.y)) return 'r';
	if (v.x <= -abs(v.y)) return 'l';
	if (v.y >= +abs(v.x)) return 'd';
	if (v.y <= -abs(v.x)) return 'u';
	return ' '; // This should never happen.
}

bool is_convex(const std::vector<sf::Vector2f>& polygon)
{
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

std::vector<std::array<sf::Vector2f, 3>> triangulate(const std::vector<sf::Vector2f>& polygon)
{
	//Ear clipping algorithm: https://www.youtube.com/watch?v=d9tytAQbpXM
	size_t vertex_count = polygon.size();
	assert(vertex_count >= 3);
	if (vertex_count == 3) {
		return { { polygon[0], polygon[1], polygon[2] } };
	}
	bool is_polygon_clockwise = false;
	{
		size_t i1 = std::ranges::max_element(polygon, std::less{}, &sf::Vector2f::x) - polygon.begin();
		size_t i0 = (i1 + vertex_count - 1) % vertex_count;
		size_t i2 = (i1 + 1) % vertex_count;
		sf::Vector2f v10 = polygon[i0] - polygon[i1];
		sf::Vector2f v12 = polygon[i2] - polygon[i1];
		is_polygon_clockwise = is_clockwise(v10, v12);
	}
	std::vector<float> angles(vertex_count);
	for (size_t i = 0; i < vertex_count; ++i) {
		size_t i0 = (i + vertex_count - 1) % vertex_count;
		size_t i1 = i;
		size_t i2 = (i + 1) % vertex_count;
		sf::Vector2f v10 = polygon[i0] - polygon[i1];
		sf::Vector2f v12 = polygon[i2] - polygon[i1];
		float angle = angle_unsigned(v10, v12);
		if (is_clockwise(v10, v12) != is_polygon_clockwise)
			angle = M_2PI - angle;
		angles[i1] = angle;
	}
	std::vector<sf::Vector2f> polygon_copy = polygon;
	std::vector<std::array<sf::Vector2f, 3>> triangles;
	while (vertex_count > 3) {
		size_t i2 = std::ranges::min_element(angles) - angles.begin();
		size_t i0 = (i2 + vertex_count - 2) % vertex_count;
		size_t i1 = (i2 + vertex_count - 1) % vertex_count;
		size_t i3 = (i2 + 1) % vertex_count;
		size_t i4 = (i2 + 2) % vertex_count;
		triangles.push_back({ polygon_copy[i1], polygon_copy[i2], polygon_copy[i3] });
		{
			sf::Vector2f v10 = polygon_copy[i0] - polygon_copy[i1];
			sf::Vector2f v13 = polygon_copy[i3] - polygon_copy[i1];
			float angle = angle_unsigned(v10, v13);
			if (is_clockwise(v10, v13) != is_polygon_clockwise)
				angle = M_2PI - angle;
			angles[i1] = angle;
		}
		{
			sf::Vector2f v31 = polygon_copy[i1] - polygon_copy[i3];
			sf::Vector2f v34 = polygon_copy[i4] - polygon_copy[i3];
			float angle = angle_unsigned(v31, v34);
			if (is_clockwise(v31, v34) != is_polygon_clockwise)
				angle = M_2PI - angle;
			angles[i3] = angle;
		}
		polygon_copy.erase(polygon_copy.begin() + i2);
		angles.erase(angles.begin() + i2);
		--vertex_count;
	}
	triangles.push_back({ polygon_copy[0], polygon_copy[1], polygon_copy[2] });
	return triangles;
}
