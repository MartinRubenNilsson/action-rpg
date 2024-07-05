#include "stdafx.h"
#include "fonts.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include "console.h"

namespace fonts
{
	struct Font
	{
		std::vector<unsigned char> data;
		stbtt_fontinfo info{};
	};

	std::vector<Font> _fonts;
	std::unordered_map<std::string, FontHandle> _font_path_to_handle;

	FontHandle load_font(const std::string& path)
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
			return FontHandle::Invalid;
		}

		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<unsigned char> buffer(size);
		file.read(reinterpret_cast<char*>(buffer.data()), size);

		stbtt_fontinfo info{};
		if (!stbtt_InitFont(&info, buffer.data(), 0)) {
			console::log_error("Failed to load font: " + path);
			return FontHandle::Invalid;
		}

		const FontHandle handle = (FontHandle)_fonts.size();

		Font& font = _fonts.emplace_back();
		font.data = std::move(buffer);
		font.info = info;

		_font_path_to_handle[path] = handle;

		return handle;
	}
}