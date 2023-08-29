#pragma once

namespace ecs
{
	bool player_exists();
	entt::entity get_player_entity();
	sf::Vector2f get_player_position();

	void update_player();
}
