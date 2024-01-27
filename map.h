#pragma once

namespace map
{
	// Calling open(), close(), or reset() will not immediately change the map.
	// Instead, the change will be queued and will take effect when update() is called.

	bool open(const std::string& map_name, bool reset_if_already_open = false);
	void close();
	void reset();
	void update();

	std::string get_name();
	sf::FloatRect get_bounds();

	// Sets the name of the entity that should be used as the player's starting position.
	// You may call this function before or after open(), as long as you call it before update().
	void set_player_spawnpoint(const std::string& entity_name);

	bool play_footstep_sound_at(const sf::Vector2f& position);
}
