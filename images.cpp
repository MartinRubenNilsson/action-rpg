#include "stdafx.h"
#include "images.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace images
{
	unsigned char* load(const char* filename, int* width, int* height, int* channels, int desired_channels)
	{
		return stbi_load(filename, width, height, channels, desired_channels);
	}

	void free(unsigned char* data)
	{
		stbi_image_free(data);
	}

	const char* failure_reason()
	{
		return stbi_failure_reason();
	}
}