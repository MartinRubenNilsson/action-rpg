#pragma once

namespace map
{
	// Loads all TMX maps in the assets/maps directory.
	// Safe to call multiple times.
	void load_assets();

	// Returns an internal registry containing the deserialized version of the currently open map.
	entt::registry& get_registry();

	// Deserializes the map with the given filename into the internal registry.
	// If a map is already open, it is closed first.
	// Returns true if successful, false otherwise.
	// Example: open("dungeon.tmx")
	bool open(const std::string& filename);

	// Clears the internal registry.
	void close();
}
