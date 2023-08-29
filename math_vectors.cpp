#include "math_vectors.h"
#include <box2d/b2_math.h>

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
	return v / length(v);
}

sf::Vector2f normalize_safe(const sf::Vector2f& v) {
	if (float l = length(v)) return v / l;
	return sf::Vector2f(0, 0);
}

float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
	return a.x * b.x + a.y * b.y;
}

float cross(const sf::Vector2f& a, const sf::Vector2f& b) {
	return a.x * b.y - a.y * b.x;
}

float angle_between(const sf::Vector2f& a, const sf::Vector2f& b) {
	return std::acos(dot(a, b) / (length(a) * length(b))); // TODO: Check for division by zero
}

sf::Vector2f rotate(const sf::Vector2f& v, float angle) {
	float c = std::cos(angle);
	float s = std::sin(angle);
	return sf::Vector2f(v.x * c - v.y * s, v.x * s + v.y * c);
}

sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t) {
	return a + (b - a) * t;
}

char get_direction(const sf::Vector2f& v)
{
	if (v.x >=  abs(v.y)) return 'r';
	if (v.x <= -abs(v.y)) return 'l';
	if (v.y >=  abs(v.x)) return 'd';
	return 'u';
}

void set_position(b2Body* body, const sf::Vector2f& position) {
	body->SetTransform(vector_cast<b2Vec2>(position), body->GetAngle());
}

sf::Vector2f get_position(const b2Body* body) {
	return vector_cast<sf::Vector2f>(body->GetPosition());
}

void set_linear_velocity(b2Body* body, const sf::Vector2f& velocity) {
	body->SetLinearVelocity(vector_cast<b2Vec2>(velocity));
}

sf::Vector2f get_linear_velocity(const b2Body* body) {
	return vector_cast<sf::Vector2f>(body->GetLinearVelocity());
}

float get_linear_speed(const b2Body* body) {
	return length(get_linear_velocity(body));
}
