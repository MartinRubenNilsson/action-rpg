#include "stdafx.h"
#include "images.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace images {
	unsigned char* load(
		const char* filename,
		unsigned int* width,
		unsigned int* height,
		unsigned int* channels_in_file,
		unsigned int desired_channels
	) {
		return stbi_load(
			filename,
			(int*)width,
			(int*)height,
			(int*)channels_in_file,
			(int)desired_channels
		);
	}

	void free(unsigned char* retval_from_load) {
		stbi_image_free(retval_from_load);
	}

	const char* failure_reason() {
		return stbi_failure_reason();
	}
}