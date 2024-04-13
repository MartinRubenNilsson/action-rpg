#pragma once
#include "timer.h"

namespace ecs
{
	//TODO: refactor into PlayerInputFlags. except axis?
	struct PlayerInput
	{
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
		bool run = false;
		bool stealth = false;
		bool interact = false;
		bool shoot_bow = false; 
		bool drop_bomb = false;
		bool use_sword = false;
	};

	enum class PlayerState
	{
		Normal,
		SwingingSword,
		ShootingBow,
		Dying,
		Dead
	};

	struct Player
	{
		PlayerInput input;
		PlayerState state = PlayerState::Normal;
		sf::Vector2f look_dir = { 0.f, 1.f };
		Timer hurt_timer = { 1.f };
		int max_health = 3;
		int health = 3;
		int arrows = 10;
		int bombs = 5;
		int rupees = 10;
		entt::entity held_item = entt::null;
	};

	void process_event_players(const sf::Event& ev);
	void update_players(float dt);
	void debug_draw_players();

	void emplace_player(entt::entity entity, const Player& player = Player());
	bool remove_player(entt::entity entity);

	bool kill_player(entt::entity entity);
}
