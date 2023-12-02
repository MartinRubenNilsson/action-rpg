#pragma once

namespace ecs
{
	struct PlayerInput
	{
		sf::Vector2f direction;
		bool run = false;
		bool interact = false;
	};

	struct PlayerState
	{
		sf::Vector2f direction; // normalized
		int health = 3;
		float hurt_cooldown_timer = 0.f; // time left until player can be hurt again
	};

	struct Player
	{
		PlayerInput input;
		PlayerState state;
	};

	void process_event_player(const sf::Event& event);
	void update_player(float dt);

	void emplace_player(entt::entity entity, const Player& player);
	bool player_exists();
	sf::Vector2f get_player_center();

	void hurt_player(int health_to_remove);
}
