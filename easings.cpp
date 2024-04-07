#include "stdafx.h"
#include "easings.h"

constexpr float PI = 3.14159265f;

float ease_in_sine(float x) {
	return 1.f - cosf(x * PI / 2.f);
}
float ease_out_sine(float x) {
	return sinf(x * PI / 2.f);
}
float ease_in_out_sine(float x) {
	return -(cosf(PI * x) - 1.f) / 2.f;
}
float ease_in_quad(float x) {
	return x * x;
}
float ease_out_quad(float x) {
	return 1.f - (1.f - x) * (1.f - x);
}
float ease_in_out_quad(float x) {
	return x < 0.5f ? 2.f * x * x : 1.f - powf(-2.f * x + 2.f, 2) / 2.f;
}
float ease_in_expo(float x) {
	return x == 0.f ? 0.f : powf(2.f, 10.f * x - 10.f);
}
float ease_out_expo(float x) {
	return x == 1.f ? 1.f : 1.f - powf(2.f, -10.f * x);
}
float ease_in_out_expo(float x) {
	if (x == 0.f) return 0.f;
	if (x == 1.f) return 1.f;
	if (x < 0.5f) return powf(2.f, 20.f * x - 10.f) / 2.f;
	return (2.f - powf(2.f, -20.f * x + 10.f)) / 2.f;
}

