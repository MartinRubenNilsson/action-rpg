#pragma once
#include "timer.h"

namespace ecs
{
	struct PlayerInput
	{
		int axis_x = 0;
		int axis_y = 0;
		bool run = false;
		bool stealth = false;
		bool interact = false;
		bool projectile_attack = false; 
	};

	struct Player
	{
		PlayerInput input;
		Timer hurt_timer = { 1.f };
		Timer kill_timer = { 1.f };
		sf::Vector2f facing_direction = { 0.f, 1.f }; // The direction the player is facing
		int health = 3;
		int arrow_ammo = 10;
	};

	void process_event_player(const sf::Event& event);
	void update_player(float dt);

	void emplace_player(entt::entity entity, const Player& player);
	void remove_player(entt::entity entity);

	bool kill_player(entt::entity entity);
	bool hurt_player(entt::entity entity, int health_to_remove);
}
