#pragma once

namespace map
{
	void load_assets(); // Safe to call multiple times.

	// map_filename is not a path, just the filename including the .tmx extension.
	bool create_entities(entt::registry& registry, const std::string& map_filename);
	void update_animated_tiles(entt::registry& registry, float dt);
}
