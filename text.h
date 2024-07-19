#pragma once

namespace text
{
	struct Text
	{
		Handle<fonts::Font> font;
		std::u32string utf32_string;
		float pixel_height = 30.f;
		float letter_spacing_factor = 1.f;
		float line_spacing_factor = 1.f;
		Vector2f position;
	};

	void render(const Text& text);
}
