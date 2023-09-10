#pragma once

namespace audio
{
	void initialize();
	void shutdown();
	void update();

	// Loads all FMOD banks in the assets/audio/banks directory.
	void load_banks();

	// Creates and starts a new FMOD event instance with the given path.
	// If the event is not one-shot, it will need to be stopped manually.
	void play(const std::string& event_path);

	// Stops all FMOD event instances.
	void stop_all();
}

