#include "stdafx.h"
#include "random.h"
#include <random>
#include <noise/noise.h>

namespace random
{
	const float PI = 3.14159265359f;
	const float PI_2 = 6.28318530718f;

	std::default_random_engine _engine(std::random_device{}());
	noise::module::Perlin _perlin;

	bool chance(float probability_of_true)
	{
		std::bernoulli_distribution dist(probability_of_true);
		return dist(_engine);
	}

	float uniform_f(float min, float max)
	{
		std::uniform_real_distribution<float> dist(min, max);
		return dist(_engine);
	}

	int uniform_i(int min, int max)
	{
		std::uniform_int_distribution<int> dist(min, max);
		return dist(_engine);
	}

	sf::Vector2f on_circle(float radius)
	{
		float angle = uniform_f(0, PI_2);
		return sf::Vector2f(cos(angle) * radius, sin(angle) * radius);
	}

	sf::Vector2f in_circle(float radius)
	{
		// http://mathworld.wolfram.com/DiskPointPicking.html
		float r = sqrt(uniform_f(0.0f, radius));
		return on_circle(r);
	}

	double perlin_noise_d(double x, double y, double z) {
		return _perlin.GetValue(x, y, z);
	}

	float perlin_noise_f(float x, float y, float z) {
		return (float)_perlin.GetValue((double)x, (double)y, (double)z);
	}
}