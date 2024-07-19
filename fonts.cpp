#include "stdafx.h"
#include "fonts.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include "pool.h"
#include "console.h"

namespace fonts
{
	struct Font
	{
		std::string path;
		std::vector<unsigned char> data;
		stbtt_fontinfo info{};
	};

	Pool<Font> _font_pool;
	std::unordered_map<std::string, Handle<Font>> _font_path_to_handle;

	Handle<Font> load_font(const std::string& path)
	{
		// CHECK IF FONT ALREADY LOADED

		auto it = _font_path_to_handle.find(path);
		if (it != _font_path_to_handle.end()) {
			return it->second;
		}

		// LOAD FONT

		std::ifstream file(path, std::ios::binary);
		if (!file) {
			console::log_error("Failed to open font file: " + path);
			return Handle<Font>();
		}

		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		file.seekg(0, std::ios::beg);

		Font font{};
		font.path = path;
		font.data.resize(size);
		file.read(reinterpret_cast<char*>(font.data.data()), size);

		stbtt_fontinfo info{};
		if (!stbtt_InitFont(&font.info, font.data.data(), 0)) {
			console::log_error("Failed to load font: " + path);
			return Handle<Font>();
		}

		// IMPORTANT: We must move construct the font, otherwise
		// font.data is reallocated and font.info invalidated.
		const Handle<Font> handle = _font_pool.emplace(std::move(font));
		_font_path_to_handle[path] = handle;

		return handle;
	}
}