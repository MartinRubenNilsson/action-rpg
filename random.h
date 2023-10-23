#pragma once

namespace random
{
	// UNIFORM DISTRIBUTIONS

	float range_f(float min, float max);
	int range_i(int min, int max); // inclusive
	sf::Vector2f on_circle(float radius);
	sf::Vector2f in_circle(float radius);
}
