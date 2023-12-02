#pragma once

namespace map
{
	// Requests that the map with the given name be opened.
	// The map will not be opened until update() is called.
	// If the map is already open, it will not be re-opened unless force is true.
	// Returns true if the map was found, false otherwise.
	bool open(const std::string& map_name, bool force = false);

	// Requests that the current map be closed.
	// The map will not be closed until update() is called.
	void close();

	// Requests that the current map be reset.
	// The map will not be reset until update() is called.
	void reset();

	// Executes the requests made by open(), close(), and reset().
	// This function should be called once per frame.
	void update();

	// Returns the name of the current map.
	// If no map is open, returns an empty string.
	std::string get_name();

	// Returns the bounds of the current map in world space units (meters).
	// If no map is open, returns an empty rectangle.
	sf::FloatRect get_bounds();

	struct SpawnOptions
	{
		// Special value that means "use the object's default value".
		static const float DEFAULT;

		float x = DEFAULT; // x-position in meters
		float y = DEFAULT; // y-position in meters
		float z = DEFAULT; // z-position in layers; higher z-values are drawn on top of lower z-values.
		float w = DEFAULT; // width in meters
		float h = DEFAULT; // height in meters
	};

	// Creates an entity from the given template object and options.
	// Returns entt::null if the template was not found.
	entt::entity spawn(std::string template_name, const SpawnOptions& options);

	// Sets the name of the entity that should be used as
	// the player's starting position when a map is opened.
	// You may call this function before or after open(),
	// as long as you call it before update().
	void set_spawnpoint(const std::string& entity_name);
}
