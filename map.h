#pragma once

namespace map
{
	// Calling open(), close(), or reset() will not immediately change the map.
	// Instead, the change will be queued and will take effect when update() is called.

	void open(const std::string& map_name, bool reset_if_open = false);
	void close();
	void reset();
	void update();

	std::string get_name();
	sf::IntRect get_tile_bounds();
	sf::FloatRect get_world_bounds();

	std::vector<sf::Vector2f> pathfind(const sf::Vector2f& start, const sf::Vector2f& end);

	bool play_footstep_sound_at(const sf::Vector2f& position);
}
