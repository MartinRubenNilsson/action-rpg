#pragma once

namespace fonts
{
	struct Font;

	struct Glyph
	{
		// All values are in unscaled coordinates.
		// Positive x-axis is to the right, positive y-axis is up.

		int advance_width = 0;
		int left_side_bearing = 0;
		int x0 = 0;
		int y0 = 0;
		int x1 = 0;
		int y1 = 0;
	};

	Handle<Font> load_font(const std::string& path);
	int get_line_spacing(Handle<Font> handle); // in unscaled coordinates
	float get_scale_for_pixel_height(Handle<Font> handle, float pixel_height);
	Glyph get_glyph(Handle<Font> handle, char32_t codepoint);
	int get_kerning_advance(Handle<Font> handle, char32_t codepoint1, char32_t codepoint2); // in unscaled coordinates
}

