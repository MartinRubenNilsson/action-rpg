#pragma once

namespace text {
	struct Text {
		Handle<fonts::Font> font;
		std::u32string unicode_string; // String of unicode codepoints; can be created using the U"..." string literal.
		float pixel_height = 30.f;
		float letter_spacing_factor = 1.f;
		float line_spacing_factor = 1.f;
		Vector2f position;
		Vector2f scale = { 1.f, 1.f };
	};

	void render(const Text& text);
}
