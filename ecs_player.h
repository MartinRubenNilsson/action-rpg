#pragma once
#include "timer.h"

namespace ecs
{
	enum class PlayerState
	{
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
		bool fire_arrow = false; 
		bool drop_bomb = false;
		bool sword_attack = false;
	};

	struct Player
	{
		PlayerInput input;
		PlayerState state = PlayerState::Normal;
		sf::Vector2f look_dir = { 0.f, 1.f };
		Timer hurt_timer = { 1.f };
		Timer bow_shot_timer = { 0.66f };
		Timer sword_attack_timer = { 0.18f };
		int health = 3;
		int arrows = 10;
		int bombs = 5;
		int rupees = 10;
	};

	void process_event_players(const sf::Event& ev);
	void update_players(float dt);
	void debug_draw_players();

	void emplace_player(entt::entity entity, const Player& player = Player());
	bool remove_player(entt::entity entity);

	bool kill_player(entt::entity entity);
}
