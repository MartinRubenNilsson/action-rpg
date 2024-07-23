#include "stdafx.h"
#include "fonts.h"
#include "pool.h"
#include "console.h"
#include "graphics.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace fonts
{
	const int ATLAS_TEXTURE_SIZE = 1024;

	struct Font
	{
		std::vector<unsigned char> data;
		stbtt_fontinfo info{};
		int ascent = 0; // The (unscaled) coordinate above the baseline the font extends.
		int descent = 0; // The (unscaled) coordinate below the baseline the font extends; typically negative.
		int line_gap = 0; // The (unscaled) spacing between one row's descent and the next row's ascent.

		std::vector<unsigned char> atlas_pixels; // size = ATLAS_TEXTURE_SIZE * ATLAS_TEXTURE_SIZE
		stbtt_pack_context pack_context{};
		std::unordered_map<char32_t, stbtt_packedchar> packed_chars;
		Handle<graphics::Texture> atlas_texture;
		bool atlas_texture_needs_updating = true;
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
		font.data.resize(size);
		file.read(reinterpret_cast<char*>(font.data.data()), size);

		stbtt_fontinfo info{};
		if (!stbtt_InitFont(&font.info, font.data.data(), 0)) {
			console::log_error("Failed to load font: " + path);
			return Handle<Font>();
		}

		stbtt_GetFontVMetrics(&font.info, &font.ascent, &font.descent, &font.line_gap);

		font.atlas_pixels.resize(ATLAS_TEXTURE_SIZE * ATLAS_TEXTURE_SIZE);
		stbtt_PackBegin(&font.pack_context, font.atlas_pixels.data(), ATLAS_TEXTURE_SIZE, ATLAS_TEXTURE_SIZE, 0, 1, nullptr);
		font.atlas_texture = graphics::create_texture(ATLAS_TEXTURE_SIZE, ATLAS_TEXTURE_SIZE, 1, font.atlas_pixels.data());
		graphics::set_texture_filter(font.atlas_texture, graphics::TextureFilter::Linear);

		// IMPORTANT: We must move construct the font, otherwise
		// font.data/font.atlas_pixels are reallocated and
		// font.info/font.pack_context are invalidated.
		const Handle<Font> handle = _font_pool.emplace(std::move(font));
		_font_path_to_handle[path] = handle;

		return handle;
	}

	Handle<graphics::Texture> get_atlas_texture(Handle<Font> handle)
	{
		Font* font = _font_pool.get(handle);
		if (!font) return Handle<graphics::Texture>();
		if (font->atlas_texture_needs_updating) {
			graphics::update_texture(font->atlas_texture, font->atlas_pixels.data());
			font->atlas_texture_needs_updating = false;
		}
		return font->atlas_texture;
	}

	int get_line_spacing(Handle<Font> handle)
	{
		const Font* font = _font_pool.get(handle);
		if (!font) return 0;
		return font->ascent - font->descent + font->line_gap;
	}

	float get_scale_for_pixel_height(Handle<Font> handle, float pixel_height)
	{
		const Font* font = _font_pool.get(handle);
		if (!font) return 0.0f;
		return stbtt_ScaleForPixelHeight(&font->info, pixel_height);
	}

	Glyph get_glyph(Handle<Font> handle, char32_t codepoint)
	{
		Font* font = _font_pool.get(handle);
		if (!font) return Glyph();
		Glyph glyph{};
		stbtt_GetCodepointHMetrics(&font->info, codepoint, &glyph.advance_width, &glyph.left_side_bearing);
		stbtt_GetCodepointBox(&font->info, codepoint, &glyph.x0, &glyph.y0, &glyph.x1, &glyph.y1);
		auto packed_char_it = font->packed_chars.find(codepoint);
		if (packed_char_it == font->packed_chars.end()) {
			const float font_size = 30.f; //Hardcoded for now
			stbtt_packedchar packed_char{};
			stbtt_PackFontRange(&font->pack_context, font->data.data(), 0, font_size, codepoint, 1, &packed_char);
			packed_char_it = font->packed_chars.emplace(codepoint, packed_char).first;
			font->atlas_texture_needs_updating = true;
		}
		glyph.s0 = packed_char_it->second.x0;
		glyph.t0 = packed_char_it->second.y0;
		glyph.s1 = packed_char_it->second.x1;
		glyph.t1 = packed_char_it->second.y1;
		return glyph;
	}

	int get_kerning_advance(Handle<Font> handle, char32_t codepoint1, char32_t codepoint2)
	{
		const Font* font = _font_pool.get(handle);
		if (!font) return 0;
		return stbtt_GetCodepointKernAdvance(&font->info, codepoint1, codepoint2);
	}
}