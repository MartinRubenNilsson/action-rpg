#pragma once

class Clock
{
public:
	double get_elapsed_time() const;
	double restart();

private:
	using ClockType = std::chrono::steady_clock;

	ClockType::time_point _start_time = ClockType::now();
};

