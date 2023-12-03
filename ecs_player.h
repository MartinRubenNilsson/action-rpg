#pragma once

class Timer
{
public:
	Timer() = default;
	Timer(float duration); // Doesn't start the timer.

	void start();
	void stop();
	bool update(float dt); // Returns true if the timer finished this frame.
	bool finished() const;
	float get_progress() const; // 0.f = just started, 1.f = finished
	float get_duration() const { return duration; }
	float get_time_left() const { return time_left; }
	float get_time_elapsed() const { return duration - time_left; }

private:
	float duration = 0.f;
	float time_left = 0.f;
};

namespace ecs
{
	extern const float PLAYER_HURT_COOLDOWN_DURATION;
	extern const float PLAYER_STEP_COOLDOWN_DURATION;

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
	};

	struct Player
	{
		PlayerInput input;
		PlayerState state;
		Timer hurt_timer = { PLAYER_HURT_COOLDOWN_DURATION };
		Timer step_timer = { PLAYER_STEP_COOLDOWN_DURATION };
	};

	void process_event_player(const sf::Event& event);
	void update_player(float dt);

	void emplace_player(entt::entity entity, const Player& player);
	bool player_exists();
	sf::Vector2f get_player_center();

	void kill_player();
	void hurt_player(int health_to_remove);
}
