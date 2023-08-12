#pragma once

namespace map
{
	void load_assets(); // Safe to call multiple times.
	bool load(entt::registry& registry, const std::string& name);
}
