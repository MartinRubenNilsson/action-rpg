#pragma once

namespace ecs
{
	bool player_exists();
	entt::entity get_player_entity();
	sf::Vector2f get_player_center();

	void process_event_player(const sf::Event& event);
	void update_player();
}
