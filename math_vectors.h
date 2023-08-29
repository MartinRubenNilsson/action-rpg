#pragma once
#include <SFML/System/Vector2.hpp>

template <class U, class V>
U vector_cast(const V& v) {
	return U(v.x, v.y);
}

sf::Vector2f operator*(const sf::Vector2f& left, const sf::Vector2f& right);
sf::Vector2f operator/(const sf::Vector2f& left, const sf::Vector2f& right);

bool is_zero(const sf::Vector2f& v);
float length_squared(const sf::Vector2f& v);
float length(const sf::Vector2f& v);
sf::Vector2f normalize(const sf::Vector2f& v);
sf::Vector2f normalize_safe(const sf::Vector2f& v); // Returns zero vector if v is zero
float dot(const sf::Vector2f& a, const sf::Vector2f& b); // Dot product
float cross(const sf::Vector2f& a, const sf::Vector2f& b); // 2D cross product, aka determinant
float angle_between(const sf::Vector2f& a, const sf::Vector2f& b); // Returns angle in radians
sf::Vector2f rotate(const sf::Vector2f& v, float angle); // Returns a copy of v rotated by angle radians
sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t); // Linear interpolation between a and b

// Returns the direction (one of 'u', 'd', 'l', 'r') in which v roughly points.
// Behavior is undefined if v is zero.
char get_direction(const sf::Vector2f& v);

class b2Body;
void set_position(b2Body* body, const sf::Vector2f& position);
sf::Vector2f get_position(const b2Body* body);
void set_linear_velocity(b2Body* body, const sf::Vector2f& velocity);
sf::Vector2f get_linear_velocity(const b2Body* body);
float get_linear_speed(const b2Body* body);

