#pragma once
#include "timer.h"

namespace ecs
{
	enum class PlayerState {
		Normal,
		Attacking,
		Dying,
		Dead
	};

	struct PlayerInput
	{
		int axis_x = 0;
		int axis_y = 0;
		bool run = false;
		bool stealth = false;
		bool interact = false;
		bool arrow_attack = false; 
	};

	struct Player
	{
		PlayerInput input;
		PlayerState state = PlayerState::Normal;
		Timer hurt_timer = { 1.f };
		Timer kill_timer = { 1.f };
		Timer bow_shot_timer = { 0.66f };
		sf::Vector2f facing_direction = { 0.f, 1.f }; // The direction the player is facing
		int health = 3;
		int arrow_ammo = 10;
	};

	void process_event_player(const sf::Event& ev);
	void update_player(float dt);
	void debug_player();

	void emplace_player(entt::entity entity, const Player& player = Player());
	void remove_player(entt::entity entity);

	bool kill_player(entt::entity entity);
	bool hurt_player(entt::entity entity, int health_to_remove);
}
