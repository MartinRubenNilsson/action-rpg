#pragma once

namespace text
{
	struct Text
	{
		fonts::FontHandle font = fonts::FontHandle::Invalid;
		std::string string;
		Vector2f position;
	};

	void render(const Text& text);
}
