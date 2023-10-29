#include "stdafx.h"
#include "random_noise.h"
#include <noise/noise.h>

namespace random
{
	noise::module::Perlin _perlin;

	double perlin_noise_d(double x, double y, double z) {
		return _perlin.GetValue(x, y, z);
	}

	float perlin_noise_f(float x, float y, float z) {
         return (float)_perlin.GetValue((double)x, (double)y, (double)z);
	}
}