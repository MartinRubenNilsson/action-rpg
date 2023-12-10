#include "stdafx.h"
#include "timer.h"

Timer::Timer(float duration)
	: _duration(std::max(0.f, duration))
	, _time(_duration)
{}

bool Timer::update(float dt, bool loop)
{
	if (_time < _duration) {
		_time += std::clamp(dt, 0.f, _duration);
		if (_time >= _duration) {
			if (loop) {
				_time -= _duration;
			} else {
				_time = _duration;
			}
			return true;
		}
	}
	return false;
}

void Timer::start() {
	_time = 0.f;
}

void Timer::stop() {
	_time = _duration;
}

float Timer::get_progress() const {
	return _duration ? _time / _duration : 1.f;
}