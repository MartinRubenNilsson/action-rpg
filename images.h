#pragma once

// images.h - Wrapper for stb_image.h

namespace images
{
	unsigned char* load(const char* filename, int* width, int* height, int* channels, int desired_channels = 0);
	void free(unsigned char* data);
	const char* failure_reason();
}