#pragma once

namespace fonts
{
	struct Font;

	struct Glyph
	{
		int advance_width = 0; // in unscaled coordinates
		int left_side_bearing = 0; // in unscaled coordinates
	};

	Handle<Font> load_font(const std::string& path);
	int get_line_spacing(Handle<Font> handle); // in unscaled coordinates
	float get_scale_for_pixel_height(Handle<Font> handle, float pixel_height);
	Glyph get_glyph(Handle<Font> handle, char32_t codepoint);
}

