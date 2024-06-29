#pragma once

namespace settings
{
	extern bool fullscreen;
	extern unsigned int window_scale;
	extern bool vsync;
	extern float volume_master;
	extern float volume_music;
	extern float volume_sound;

	// Call apply() to make any changes take effect.
	// Also, the load functions doesn't call apply() automatically,
	// rather they only update the above variables.

	void apply();
	void save_to_stream(std::ostream& os);
	void load_from_stream(std::istream& is);
	bool save_to_file(const std::string& filename = "settings.txt");
	bool load_from_file(const std::string& filename = "settings.txt");
}
