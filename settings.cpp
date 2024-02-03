#include "stdafx.h"
#include "settings.h"
#include "window.h"
#include "audio.h"

void Settings::set() const
{
	window::Desc window_desc = window::get_desc();
	window_desc.fullscreen = fullscreen;
	window_desc.scale = window_scale;
	window_desc.vsync = vsync;
	window::create_or_update(window_desc);
	audio::set_bus_volume(audio::BUS_MASTER, volume_master);
	audio::set_bus_volume(audio::BUS_MUSIC, volume_music);
	audio::set_bus_volume(audio::BUS_SOUND, volume_sound);
}

void Settings::get()
{
	fullscreen = window::get_desc().fullscreen;
	vsync = window::get_desc().vsync;
	window_scale = window::get_desc().scale;
	volume_master = audio::get_bus_volume(audio::BUS_MASTER);
	volume_music = audio::get_bus_volume(audio::BUS_MUSIC);
	volume_sound = audio::get_bus_volume(audio::BUS_SOUND);
}

void Settings::write(std::ostream& os) const
{
	os << "fullscreen " << fullscreen << std::endl;
	os << "window_scale " << window_scale << std::endl;
	os << "vsync " << vsync << std::endl;
	os << "volume_master " << volume_master << std::endl;
	os << "volume_music " << volume_music << std::endl;
	os << "volume_sound " << volume_sound << std::endl;
}

void Settings::read(std::istream& is)
{
	*this = Settings{};
	std::string line;
	while (std::getline(is, line)) {
		std::istringstream iss(line);
		std::string key;
		iss >> key;
		if (key == "fullscreen") iss >> fullscreen;
		else if (key == "window_scale") iss >> window_scale;
		else if (key == "vsync") iss >> vsync;
		else if (key == "volume_master") iss >> volume_master;
		else if (key == "volume_music") iss >> volume_music;
		else if (key == "volume_sound") iss >> volume_sound;
	}
}

bool Settings::save(const std::filesystem::path& filename) const
{
	std::ofstream ofs(filename);
	if (!ofs) return false;
	write(ofs);
	return true;
}

bool Settings::load(const std::filesystem::path& filename)
{
	std::ifstream ifs(filename);
	if (!ifs) return false;
	read(ifs);
	return true;
}
