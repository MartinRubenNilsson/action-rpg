#pragma once

class Timer
{
public:
	Timer() = default;
	Timer(float duration); // Doesn't automatically start the timer.

	void start();
	void stop();
	bool update(float dt); // Returns true if the timer finished this frame.
	float get_progress() const; // 0.f = just started, 1.f = finished
	float get_duration() const { return duration; }
	float get_time_left() const { return time_left; }
	float get_time_elapsed() const { return duration - time_left; }
	bool is_running() const { return time_left > 0.f; }
	bool is_finished() const { return time_left == 0.f; }

private:
	float duration = 0.f;
	float time_left = 0.f;
};

