#pragma once

namespace audio
{
	void initialize();
	void shutdown();
	void update();

	// Loads all FMOD banks in the assets/audio/banks directory.
	void load_banks();

	void play(const std::string& event_path);
}

