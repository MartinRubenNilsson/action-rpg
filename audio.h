#pragma once

namespace audio
{
	// (Re)loads all OGG files in the assets/audio/music folder,
	// and all WAV files in the assets/audio/sounds folder.
	void load_music_and_sounds();

	// Plays the music with the given name.
	void play_music(const std::string& name);

	// Plays the sound with the given name.
	void play_sound(const std::string& name);
}

