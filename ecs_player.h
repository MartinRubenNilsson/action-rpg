#pragma once
#include "timer.h"

namespace ecs
{
	enum InputFlags : uint32_t
	{
		// Continuous actions

		INPUT_LEFT        = (1 << 1),
		INPUT_RIGHT       = (1 << 2),
		INPUT_UP          = (1 << 3),
		INPUT_DOWN        = (1 << 4),
		INPUT_RUN         = (1 << 5),
		INPUT_STEALTH     = (1 << 6),

		// One-shot actions

		INPUT_INTERACT    = (1 << 7),
		INPUT_SWING_SWORD = (1 << 8),
		INPUT_SHOOT_BOW   = (1 << 9),
		INPUT_DROP_BOMB   = (1 << 10),
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
		uint32_t input_flags = 0;
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
