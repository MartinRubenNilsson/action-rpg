#pragma once
#include "timer.h"

namespace ecs
{
	struct PlayerInput
	{
		sf::Vector2f direction;
		bool run = false;
		bool interact = false;
		bool projectile_attack = false; 
	};

	struct PlayerState
	{
		sf::Vector2f direction; // normalized
		int health = 3;
	};

	struct Player
	{
		PlayerInput input;
		PlayerState state;
		Timer hurt_timer = { 1.0f };
		Timer step_timer = { 0.3f }; // for footstep sounds
		Timer kill_timer = { 1.f };
		int arrowAmmo;
	};

	void process_event_player(const sf::Event& event);
	void update_player(float dt);

	void emplace_player(entt::entity entity, const Player& player);
	bool player_exists();
	entt::entity get_player_entity();
	sf::Vector2f get_player_world_center();

	bool kill_player(entt::entity entity);
	bool hurt_player(entt::entity entity, int health_to_remove);
}
