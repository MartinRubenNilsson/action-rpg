#pragma once

namespace random
{
	bool coin_flip(float probability_of_true = 0.5f);
	float range_f(float min, float max); // uniform
	int range_i(int min, int max); // uniform, inclusive
	sf::Vector2f on_circle(float radius);
	sf::Vector2f in_circle(float radius);
}
