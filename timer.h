#pragma once

class Timer
{
public:
	Timer() = default;
	Timer(float duration); // Doesn't automatically start the timer.

	// Returns true if the timer finished/looped this frame.
	bool update(float dt, bool loop = false);
	void start();
	void stop();
	void finish() { _time = _duration; }
	bool running() const { return _time < _duration; }
	bool finished() const { return _time == _duration; }
	float get_progress() const; // 0.f = just started, 1.f = finished
	float get_duration() const { return _duration; }
	float get_time() const { return _time; }
	float get_time_left() const { return _duration - _time; }

private:
	float _duration = 0.f;
	float _time = 0.f;
};

