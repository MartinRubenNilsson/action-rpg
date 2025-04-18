#pragma once

// images.h - Wrapper for stb_image.h

namespace images {
	unsigned char* load(
		const char* filename,
		unsigned int* width,
		unsigned int* height,
		unsigned int* channels_in_file,
		unsigned int desired_channels = 0);

	void free(unsigned char* retval_from_load);

	const char* failure_reason();
}