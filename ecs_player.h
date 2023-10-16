#pragma once

namespace ecs
{
	struct Player
	{
		// INPUTS
		sf::Vector2f input_direction;
		bool input_run = false;
		bool input_interact = false;

		// STATE
		sf::Vector2f direction;
	};

	void process_event_player(const sf::Event& event);
	void update_player(float dt);

	bool player_exists();
	sf::Vector2f get_player_center();
}
