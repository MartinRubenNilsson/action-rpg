#pragma once

namespace random
{
	bool chance(float probability_of_true = 0.5f);
	float uniform_float(float min = 0.f, float max = 1.f);
	int uniform_int(int min, int max);
	sf::Vector2f on_circle(float radius = 1.f);
	sf::Vector2f in_circle(float radius = 1.f);
}
