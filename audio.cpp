#include "audio.h"
#include <SFML/Audio.hpp>

namespace audio
{
	std::unordered_map<std::string, sf::Music> _name_to_music;
	std::unordered_map<std::string, sf::SoundBuffer> _name_to_sound_buffer;
	std::vector<sf::Sound> _sounds;

	void load_music_and_sounds()
	{
		_name_to_music.clear();
		_name_to_sound_buffer.clear();
		_sounds.clear();

		for (const auto& entry : std::filesystem::directory_iterator("assets/audio/music"))
		{
			if (entry.path().extension() != ".ogg")
				continue;
			std::string name = entry.path().stem().string();
			sf::Music& music = _name_to_music[name];
			if (!music.openFromFile(entry.path().string()))
			{
				_name_to_music.erase(name);
				continue;
			}
			music.setLoop(true);
		}
		
		for (const auto& entry : std::filesystem::directory_iterator("assets/audio/sounds"))
		{
			if (entry.path().extension() != ".wav")
				continue;
			sf::SoundBuffer sound_buffer;
			if (sound_buffer.loadFromFile(entry.path().string()))
				_name_to_sound_buffer.emplace(entry.path().stem().string(), sound_buffer);
		}
	}

	void play_music(const std::string& name)
	{
		auto it = _name_to_music.find(name);
		if (it != _name_to_music.end())
			it->second.play();
	}

	void play_sound(const std::string& name)
	{
		auto it = _name_to_sound_buffer.find(name);
		if (it != _name_to_sound_buffer.end())
			_sounds.emplace_back(it->second).play();
	}
}

