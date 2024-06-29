#include "stdafx.h"
#include "settings.h"
#include "window.h"
#include "audio.h"

namespace settings
{
	const std::string SETTINGS_FILE_DEFAULT_PATH = "settings.txt";

	bool fullscreen = false;
	uint32_t window_scale = 5;
	bool vsync = false;
	float volume_master = 1.f;
	float volume_music = 1.f;
	float volume_sound = 1.f;

	void apply() {
#if 0
		window::State window_desc = window::get_state();
		window_desc.fullscreen = fullscreen;
		window_desc.scale = window_scale;
		window_desc.vsync = vsync;
		window::set_state(window_desc);
		audio::set_bus_volume(audio::BUS_MASTER, volume_master);
		audio::set_bus_volume(audio::BUS_MUSIC, volume_music);
		audio::set_bus_volume(audio::BUS_SOUND, volume_sound);
#endif
	}

	void save_to_stream(std::ostream& os)
	{
		os << "fullscreen " << fullscreen << std::endl;
		os << "window_scale " << window_scale << std::endl;
		os << "vsync " << vsync << std::endl;
		os << "volume_master " << volume_master << std::endl;
		os << "volume_music " << volume_music << std::endl;
		os << "volume_sound " << volume_sound << std::endl;
	}

	void load_from_stream(std::istream& is)
	{
		std::string line;
		while (std::getline(is, line)) {
			std::istringstream iss(line);
			std::string key;
			iss >> key;
			if (key == "fullscreen") {
				iss >> fullscreen;
			} else if (key == "window_scale") {
				iss >> window_scale;
			} else if (key == "vsync") {
				iss >> vsync;
			} else if (key == "volume_master") {
				iss >> volume_master;
			} else if (key == "volume_music") {
				iss >> volume_music;
			} else if (key == "volume_sound") {
				iss >> volume_sound;
			}
		}
	}

	bool save_to_file(const std::string& path)
	{
		std::ofstream ofs(path);
		if (!ofs) return false;
		save_to_stream(ofs);
		return true;
	}

	bool load_from_file(const std::string& path)
	{
		std::ifstream ifs(path);
		if (!ifs) return false;
		load_from_stream(ifs);
		return true;
	}
}
