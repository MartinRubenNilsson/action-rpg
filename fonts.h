#pragma once

namespace fonts
{
	struct Font;

	Handle<Font> load_font(const std::string& path);
}

