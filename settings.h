#pragma once

namespace settings
{
	extern const std::string SETTINGS_FILE_DEFAULT_PATH;

	extern bool fullscreen;
	extern unsigned int window_scale; // Determines window size relative to WINDOW_MIN_WIDTH/HEIGHT
	extern bool vsync;
	extern float volume_master;
	extern float volume_music;
	extern float volume_sound;

	// Call apply() to make any changes take effect.
	// (The load functions don't call it automatically,
	// rather they only update the above variables.)

	void apply();
	void save_to_stream(std::ostream& os);
	void load_from_stream(std::istream& is);
	bool save_to_file(const std::string& path = SETTINGS_FILE_DEFAULT_PATH);
	bool load_from_file(const std::string& path = SETTINGS_FILE_DEFAULT_PATH);
}
