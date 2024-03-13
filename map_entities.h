#pragma once

namespace tiled
{
	struct Map;
}

namespace map
{
	void create_entities(const tiled::Map& map);
	void destroy_entities();
}

