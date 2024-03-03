#include "stdafx.h"
#include "random.h"
#include <random>
#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

namespace random
{
	const float PI = 3.14159265359f;
	const float PI_2 = 6.28318530718f;

	std::default_random_engine _engine(std::random_device{}());

	bool chance(float probability_of_true)
	{
		std::bernoulli_distribution dist(probability_of_true);
		return dist(_engine);
	}

	float range_f(float min, float max)
	{
		std::uniform_real_distribution<float> dist(min, max);
		return dist(_engine);
	}

	int range_i(int min, int max)
	{
		std::uniform_int_distribution<int> dist(min, max);
		return dist(_engine);
	}

	sf::Vector2f on_circle(float radius)
	{
		float angle = range_f(0, PI_2);
		return sf::Vector2f(cos(angle) * radius, sin(angle) * radius);
	}

	sf::Vector2f in_circle(float radius)
	{
		// http://mathworld.wolfram.com/DiskPointPicking.html
		float r = sqrt(range_f(0.0f, radius));
		return on_circle(r);
	}

	float perlin_noise(float x, float y, float z) {
		return stb_perlin_noise3(x, y, z, 0, 0, 0);
	}

	float fractal_perlin_noise(float x, float y, float z, float lacunarity, float gain, int octaves) {
		return stb_perlin_fbm_noise3(x, y, z, lacunarity, gain, octaves);
	}
}