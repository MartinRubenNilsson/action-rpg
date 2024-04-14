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
		std::string map_name;
	};

	extern bool debug;

	void update(float dt);

	bool transition(const TransitionOptions& options);
	bool open(const std::string& map_name);
	bool close();
	bool reset();
}
