#include "stdafx.h"
#include "clock.h"

double Clock::get_elapsed_time() const
{
	const ClockType::duration elapsed_time = ClockType::now() - _start_time;
	return std::chrono::duration_cast<std::chrono::duration<double>>(elapsed_time).count();
}

double Clock::restart()
{
	const ClockType::time_point now = ClockType::now();
	const ClockType::duration elapsed_time = now - _start_time;
	_start_time = now;
	return std::chrono::duration_cast<std::chrono::duration<double>>(elapsed_time).count();
}
