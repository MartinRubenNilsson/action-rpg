#pragma once

namespace map
{
	enum class TransitionType
	{
		Open, // Open a new map.
		Close, // Close the current map.
		Reset, // Reset the current map.
	};

	struct TransitionOptions
	{
		TransitionType type = TransitionType::Open;
		std::string next_map_name;
	};

	extern bool debug;

	void update(float dt);
	// The transition progress is a value between -1 and 1. It is 0 when not transitioning,
	// positive when transitioning out of a map, and negative when transitioning in to a map.
	float get_transition_progress();

	bool transition(const TransitionOptions& options);
	bool open(const std::string& next_map_name);
	bool close();
	bool reset();
}
