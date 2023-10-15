#pragma once

namespace ecs
{
	// TODO: make Player component

	bool player_exists();
	void set_player_entity(entt::entity entity);
	entt::entity get_player_entity();
	void set_player_center(const sf::Vector2f& center);
	sf::Vector2f get_player_center();

	void process_event_player(const sf::Event& event);
	void update_player();
}
