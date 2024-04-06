#include "stdafx.h"
#include "tween.h"

namespace tween
{
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
}

