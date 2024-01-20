#pragma once
#include <SFML/System/Vector2.hpp>

constexpr float M_PI      = 3.141592654f;
constexpr float M_2PI     = 6.283185307f;
constexpr float M_1DIVPI  = 0.318309886f;
constexpr float M_1DIV2PI = 0.159154943f;
constexpr float M_PIDIV2  = 1.570796327f;
constexpr float M_PIDIV4  = 0.785398163f;

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
float angle_unsigned(const sf::Vector2f& a, const sf::Vector2f& b); // In radians; safe to call on zero vectors
float angle_signed(const sf::Vector2f& a, const sf::Vector2f& b); // In radians; safe to call on zero vectors
bool is_clockwise(const sf::Vector2f& a, const sf::Vector2f& b); // Returns true if b is clockwise of a
sf::Vector2f rotate(const sf::Vector2f& v, float angle); // Angle in radians
sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t);
//TODO: damp()
sf::Vector2f clamp(const sf::Vector2f& v, const sf::Vector2f& min, const sf::Vector2f& max);
char get_direction(const sf::Vector2f& v); // Returns the direction ('r', 'l', 'u', 'd') in which v points.

std::vector<std::array<sf::Vector2f, 3>> triangulate(const std::vector<sf::Vector2f>& polygon);
