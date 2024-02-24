#pragma once

namespace random
{
	bool chance(float probability_of_true = 0.5f);
	float range_f(float min = 0.f, float max = 1.f);
	int range_i(int min, int max);
	sf::Vector2f on_circle(float radius = 1.f);
	sf::Vector2f in_circle(float radius = 1.f);
	double perlin_noise_d(double x, double y = 0.0, double z = 0.0); // Return value is between -1 and 1.
	float perlin_noise_f(float x, float y = 0.f, float z = 0.f); // Return value is between -1 and 1.
}
