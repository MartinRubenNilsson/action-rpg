#include "stdafx.h"
#include "settings.h"
#include "window.h"
#include "window_graphics.h"
#include "audio.h"
#include "filesystem.h"

namespace settings {
	const std::string SETTINGS_FILE_DEFAULT_PATH = "settings.txt";

	bool fullscreen = false;
	unsigned int window_scale = 5;
	bool vsync = false;
	float volume_master = 1.f;
	float volume_music = 1.f;
	float volume_sound = 1.f;

	void apply() {
		window::set_fullscreen(fullscreen);
		if (!fullscreen) {
			window::set_size(GAME_FRAMEBUFFER_WIDTH * window_scale, GAME_FRAMEBUFFER_HEIGHT * window_scale);
		}
#ifdef GRAPHICS_API_OPENGL
		window::set_swap_interval(vsync ? 1 : 0); //TODO: move to graphics.cpp
#endif
		audio::set_bus_volume(audio::BUS_MASTER, volume_master);
		audio::set_bus_volume(audio::BUS_MUSIC, volume_music);
		audio::set_bus_volume(audio::BUS_SOUND, volume_sound);
	}

	void save_to_stream(std::ostream& os) {
		os << "fullscreen " << fullscreen << std::endl;
		os << "window_scale " << window_scale << std::endl;
		os << "vsync " << vsync << std::endl;
		os << "volume_master " << volume_master << std::endl;
		os << "volume_music " << volume_music << std::endl;
		os << "volume_sound " << volume_sound << std::endl;
	}

	void load_from_stream(std::istream& is) {
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

	bool save_to_file(const std::string& path) {
		std::ostringstream oss;
		save_to_stream(oss);
		return filesystem::write_text_file(path, oss.str());
	}

	bool load_from_file(const std::string& path) {
		std::string text;
		if (!filesystem::read_text_file(path, text)) return false;
		std::istringstream iss(std::move(text));
		load_from_stream(iss);
		return true;
	}
}
