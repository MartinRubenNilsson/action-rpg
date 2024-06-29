#pragma once
#include "vector2.h"

// CONSTANTS

inline constexpr float M_PI      = 3.141592654f;
inline constexpr float M_2PI     = 6.283185307f;
inline constexpr float M_1DIVPI  = 0.318309886f;
inline constexpr float M_1DIV2PI = 0.159154943f;
inline constexpr float M_PIDIV2  = 1.570796327f;
inline constexpr float M_PIDIV4  = 0.785398163f;

// SCALARS

float smoothstep(float x);
float smootherstep(float x);
float lerp(float a, float b, float t);
float lerp_angle(float a, float b, float t);

// VECTORS

template <class U, class V>
U vector_cast(const V& v) {
	return U(v.x, v.y);
}

Vector2f operator*(const Vector2f& left, const Vector2f& right);
Vector2f operator/(const Vector2f& left, const Vector2f& right);
Vector2f& operator*=(Vector2f& left, const Vector2f& right);
Vector2f& operator/=(Vector2f& left, const Vector2f& right);

// All functions are safe to call on zero vectors, and all angles are in radians.

bool is_zero(const Vector2f& v);
float length_squared(const Vector2f& v);
float length(const Vector2f& v);
Vector2f unit_vector(float angle);
Vector2f normalize(const Vector2f& v);
Vector2f abs(const Vector2f& v);
Vector2f rotate_90deg(const Vector2f& v);
float dot(const Vector2f& a, const Vector2f& b); // Dot product
float det(const Vector2f& a, const Vector2f& b); // Determinant, aka 2D cross product
float angle_unsigned(const Vector2f& a, const Vector2f& b);
float angle_signed(const Vector2f& a, const Vector2f& b);
bool is_clockwise(const Vector2f& a, const Vector2f& b); // True if b is clockwise of a
Vector2f rotate(const Vector2f& v, float angle);
Vector2f min(const Vector2f& a, const Vector2f& b);
Vector2f max(const Vector2f& a, const Vector2f& b);
Vector2f lerp(const Vector2f& a, const Vector2f& b, float t);
Vector2f lerp_polar(const Vector2f& a, const Vector2f& b, float t);
Vector2f damp(const Vector2f& a, const Vector2f& b, float damping, float dt);
Vector2f clamp(const Vector2f& v, const Vector2f& min, const Vector2f& max);
char get_direction(const Vector2f& v); // Returns the direction ('r', 'l', 'u', 'd') in which v points.

// COMPUTATIONAL GEOMETRY

bool is_convex(const std::vector<Vector2f>& polygon);
std::vector<std::array<Vector2f, 3>> triangulate(const std::vector<Vector2f>& polygon);
