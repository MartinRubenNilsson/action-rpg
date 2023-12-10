#include "stdafx.h"
#include "timer.h"

Timer::Timer(float duration)
	: duration(duration)
{}

void Timer::start() {
	time_left = duration;
}

void Timer::stop() {
	time_left = 0.f;
}

bool Timer::update(float dt)
{
	dt = std::max(0.f, dt);
	if (time_left > 0.f) {
		time_left -= dt;
		if (time_left <= 0.f) {
			time_left = 0.f;
			return true;
		}
	}
	return false;
}

float Timer::get_progress() const
{
	if (duration > 0.f)
		return 1.f - time_left / duration;
	return 1.f;
}