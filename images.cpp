#include "stdafx.h"
#include "images.h"
#include "console.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define KHRONOS_STATIC
#include <ktx.h>

namespace images {

	bool _load_image(const std::string& path, Image& image) {
		int width, height, channels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (!data) {
			console::log_error("Failed to load image: " + path);
			console::log_error(stbi_failure_reason());
			return false;
		}
		image.width = width;
		image.height = height;
		image.channels = channels;
		image.data = data;
		return true;
	}

	bool _load_ktx2_image(const std::string& path, Image& image) {
		ktxTexture2* ktx_texture2 = nullptr;
		ktxResult result = ktxTexture2_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture2);
		if (result != KTX_SUCCESS) {
			console::log_error("Failed to load KTX2 texture: " + path);
			console::log_error(ktxErrorString(result));
			return false;
		}
		image.width = ktx_texture2->baseWidth;
		image.height = ktx_texture2->baseHeight;
		image.channels = ktxTexture_GetElementSize(ktxTexture(ktx_texture2));
		image.data = ktxTexture_GetData(ktxTexture(ktx_texture2));
		image._private = ktx_texture2; // Store the KTX texture for later cleanup
		return true;
	}

	bool load_image(const std::string& path, Image& image) {
		if (path.ends_with(".ktx2")) {
			return _load_ktx2_image(path, image);
		} else {
			return _load_image(path, image);
		}
	}

	void free_image(Image& image) {
		if (image._private) {
			ktxTexture_Destroy(ktxTexture(image._private));
		} else if (image.data) {
			stbi_image_free(image.data);
		}
		image = {};
	}
}