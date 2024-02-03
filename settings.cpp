#include "stdafx.h"
#include "settings.h"
#include "window.h"
#include "audio.h"

void Settings::set() const
{
	window::set_fullscreen(fullscreen);
	audio::set_bus_volume(audio::BUS_MASTER, volume_master);
	audio::set_bus_volume(audio::BUS_MUSIC, volume_music);
	audio::set_bus_volume(audio::BUS_SOUND, volume_sound);
}

void Settings::get()
{
	fullscreen = window::is_fullscreen();
	volume_master = audio::get_bus_volume(audio::BUS_MASTER);
	volume_music = audio::get_bus_volume(audio::BUS_MUSIC);
	volume_sound = audio::get_bus_volume(audio::BUS_SOUND);
}

void Settings::write(std::ostream& os) const
{
	/*os << "fullscreen " << fullscreen << std::endl;
	os << "volume_master " << volume_master << std::endl;
	os << "volume_music " << volume_music << std::endl;
	os << "volume_sound " << volume_sound << std::endl;*/
}

void Settings::read(std::istream& is)
{
	/*std::string key;
	while (is >> key) {
		if (key == "fullscreen") is >> fullscreen;
		else if (key == "volume_master") is >> volume_master;
		else if (key == "volume_music") is >> volume_music;
		else if (key == "volume_sound") is >> volume_sound;
	}*/
}
