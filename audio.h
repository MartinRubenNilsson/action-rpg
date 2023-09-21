#pragma once

namespace audio
{
	void initialize();
	void shutdown();
	void update();

	// Loads all FMOD banks in the assets/audio/banks directory.
	void load_banks();

	// Creates and starts a new event instance with the given path.
	// If the event is not one-shot, it may keep playing indefinitely.
	void play(const std::string& path);

	// Stops all event instances.
	void stop_all();
}

