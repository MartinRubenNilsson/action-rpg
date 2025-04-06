#pragma once

struct b2Vec2;

template <typename T>
struct Vector2 {
	T x{};
	T y{};

	Vector2() = default;
	Vector2(T x, T y) : x(x), y(y) {}
	Vector2(const b2Vec2& v) : x(v.x), y(v.y) {}

	template <typename U>
	operator Vector2<U>() const {
		return { static_cast<U>(x), static_cast<U>(y) };
	}

	operator b2Vec2() const {
		return { static_cast<float>(x), static_cast<float>(y) };
	}

	auto operator<=>(const Vector2<T>&) const = default;
};

template <typename T>
Vector2<T> operator-(const Vector2<T>& right) {
	return { -right.x, -right.y };
}

template <typename T>
Vector2<T>& operator+=(Vector2<T>& left, const Vector2<T>& right) {
	left.x += right.x;
	left.y += right.y;
	return left;
}

template <typename T>
Vector2<T>& operator-=(Vector2<T>& left, const Vector2<T>& right) {
	left.x -= right.x;
	left.y -= right.y;
	return left;
}

template <typename T>
Vector2<T>& operator*=(Vector2<T>& left, const Vector2<T>& right) {
	left.x *= right.x;
	left.y *= right.y;
	return left;
}

template <typename T>
Vector2<T>& operator*=(Vector2<T>& left, const T& right) {
	left.x *= right;
	left.y *= right;
	return left;
}

template <typename T>
Vector2<T>& operator/=(Vector2<T>& left, const Vector2<T>& right) {
	left.x /= right.x;
	left.y /= right.y;
	return left;
}

template <typename T>
Vector2<T>& operator/=(Vector2<T>& left, const T& right) {
	left.x /= right;
	left.y /= right;
	return left;
}

template <typename T>
Vector2<T> operator+(const Vector2<T>& left, const Vector2<T>& right) {
	return { left.x + right.x, left.y + right.y };
}

template <typename T>
Vector2<T> operator-(const Vector2<T>& left, const Vector2<T>& right) {
	return { left.x - right.x, left.y - right.y };
}

template <typename T>
Vector2<T> operator*(const Vector2<T>& left, const Vector2<T>& right) {
	return { left.x * right.x, left.y * right.y };
}

template <typename T>
Vector2<T> operator*(const Vector2<T>& left, const T& right) {
	return { left.x * right, left.y * right };
}

template <typename T>
Vector2<T> operator*(const T& left, const Vector2<T>& right) {
	return { left * right.x, left * right.y };
}

template <typename T>
Vector2<T> operator/(const Vector2<T>& left, const Vector2<T>& right) {
	return { left.x / right.x, left.y / right.y };
}

template <typename T>
Vector2<T> operator/(const Vector2<T>& left, const T& right) {
	return { left.x / right, left.y / right };
}

using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;
using Vector2f = Vector2<float>;