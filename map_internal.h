#pragma once
#include <tmxlite/Map.hpp>

namespace map
{
	void reload_textures_impl();
	void open_impl(const std::string& map_name, const tmx::Map& map);
	void close_impl();
}

