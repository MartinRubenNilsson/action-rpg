#pragma once

namespace text
{
	struct Text
	{
		fonts::FontHandle font = fonts::FontHandle::Invalid;
		std::u32string string;
		unsigned int character_size = 30;
		float letter_spacing_factor = 1.f;
		float line_spacing_factor = 1.f;
		Vector2f position;
	};

	void render(const Text& text);
}