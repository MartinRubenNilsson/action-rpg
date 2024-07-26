#pragma once

namespace tiled
{
	struct Map;
}

namespace map
{
	struct MapPatch
	{
		std::vector<entt::entity> chests_to_open;
	};

	void create_entities(const tiled::Map& map);
	void patch_entities(const MapPatch& patch);
	void destroy_entities();
}

