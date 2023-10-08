#pragma once

namespace audio
{
	// not implemented yet
	enum Bus
	{
		Bus_Music,
		Bus_Sound,
		Bus_COUNT,
	};

	void initialize();
	void shutdown();
	void update();

	// Loads all FMOD banks in the assets/audio/banks directory.
	void load_banks();

	// Returns true if an event instance with the given path exists
	// and is currently playing, false otherwise.
	bool is_playing(const std::string& path);

	// Creates and starts a new event instance with the given path.
	// If the event is not one-shot, it may keep playing indefinitely.
	void play(const std::string& path);

	// Stops all event instances with the given path.
	void stop(const std::string& path);

	// Stops all event instances.
	void stop_all();
}

