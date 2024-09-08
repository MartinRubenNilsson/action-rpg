#pragma once

// images.h - Wrapper for stb_image.h

namespace images
{
	unsigned char* load(const char* filename, int* width, int* height, int* channels_in_file, int desired_channels = 0);
	void free(unsigned char* retval_from_load);
	const char* failure_reason();
}