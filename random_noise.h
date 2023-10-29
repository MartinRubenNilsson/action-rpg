#pragma once

namespace random
{
	// Return value is between -1.0 and 1.0.
	double perlin_noise_d(double x, double y = 0.0, double z = 0.0);
	float perlin_noise_f(float x, float y = 0.0f, float z = 0.0f);
}
