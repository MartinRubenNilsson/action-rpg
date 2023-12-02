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
[[nodiscard]] sf::Vector2f normalize(const sf::Vector2f& v); // Safe to call on zero vector
float dot(const sf::Vector2f& a, const sf::Vector2f& b); // Dot product
float det(const sf::Vector2f& a, const sf::Vector2f& b); // Determinant, aka 2D cross product
float angle(const sf::Vector2f& a, const sf::Vector2f& b); // In radians; safe to call on zero vectors
float angle_signed(const sf::Vector2f& a, const sf::Vector2f& b); // In radians; safe to call on zero vectors
sf::Vector2f rotate(const sf::Vector2f& v, float angle); // Angle in radians
sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t);
sf::Vector2f clamp(const sf::Vector2f& v, const sf::Vector2f& min, const sf::Vector2f& max);

// Returns the direction (one of 'r', 'l', 'u', 'd') in which v roughly points.
// Behavior is undefined if v is zero.
char get_direction(const sf::Vector2f& v);
