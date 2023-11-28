#include "math_vectors.h"

sf::Vector2f operator*(const sf::Vector2f& left, const sf::Vector2f& right) {
	return sf::Vector2f(left.x * right.x, left.y * right.y);
}

sf::Vector2f operator/(const sf::Vector2f& left, const sf::Vector2f& right) {
	return sf::Vector2f(left.x / right.x, left.y / right.y);
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

float angle_between(const sf::Vector2f& a, const sf::Vector2f& b) {
	if (float denom = length(a) * length(b))
		return std::acos(dot(a, b) / denom);
	return 0.f;
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

sf::Vector2f clamp(const sf::Vector2f& v, const sf::Vector2f& min, const sf::Vector2f& max)
{
	return sf::Vector2f(
		std::clamp(v.x, min.x, max.x),
		std::clamp(v.y, min.y, max.y)
	);
}

char get_direction(const sf::Vector2f& v)
{
	if (v.x >=  abs(v.y)) return 'r';
	if (v.x <= -abs(v.y)) return 'l';
	if (v.y >=  abs(v.x)) return 'd';
	if (v.y <= -abs(v.x)) return 'u';
	return ' '; // This should never happen.
}
