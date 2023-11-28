#include "audio.h"
#include "fmod_studio.hpp"
#include "console.h"

#ifdef _DEBUG
#pragma comment(lib, "fmodL_vc.lib")
#pragma comment(lib, "fmodstudioL_vc.lib")
#else
#pragma comment(lib, "fmod_vc.lib")
#pragma comment(lib, "fmodstudio_vc.lib")
#endif

namespace audio
{
	const int MAX_AUDIO_CHANNELS = 512;
	FMOD::Studio::System* _system = nullptr;

	void initialize()
	{
		FMOD_RESULT result = FMOD::Studio::System::create(&_system);
		assert(result == FMOD_OK);
		result = _system->initialize(
			MAX_AUDIO_CHANNELS,
			FMOD_STUDIO_INIT_NORMAL,
			FMOD_INIT_NORMAL,
			nullptr);
		assert(result == FMOD_OK);
	}

	void shutdown()
	{
		_system->release();
		_system = nullptr;
	}

	void update() {
		_system->update();
	}

	void load_assets()
	{
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator("assets/audio/banks"))
		{
			if (!entry.is_regular_file()) continue;
			if (entry.path().extension() != ".bank") continue;
			FMOD::Studio::Bank *bank = nullptr;
			FMOD_RESULT result = _system->loadBankFile(
				entry.path().string().c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
			assert(result == FMOD_OK);
		}
	}

	bool _get_event_description(const std::string& path, FMOD::Studio::EventDescription** desc)
	{
		FMOD_RESULT result = _system->getEvent(path.c_str(), desc);
		if (result != FMOD_OK)
		{
			console::log_error("Failed to get audio event description: " + path);
			return false;
		}
		return true;
	}

	std::vector<FMOD::Studio::EventInstance*> _get_event_instances(const FMOD::Studio::EventDescription* desc)
	{
		int count = 0;
		desc->getInstanceCount(&count);
		std::vector<FMOD::Studio::EventInstance*> instances(count);
		desc->getInstanceList(instances.data(), count, &count);
		return instances;
	}

	bool is_playing(const std::string& path)
	{
		FMOD::Studio::EventDescription* event_desc = nullptr;
		if (!_get_event_description(path, &event_desc)) return false;
		for (FMOD::Studio::EventInstance *event_inst : _get_event_instances(event_desc))
		{
			FMOD_STUDIO_PLAYBACK_STATE state;
			event_inst->getPlaybackState(&state);
			if (state == FMOD_STUDIO_PLAYBACK_PLAYING)
				return true;
		}
		return false;
	}

	void play(const std::string& path)
	{
		FMOD::Studio::EventDescription* event_desc = nullptr;
		if (!_get_event_description(path, &event_desc)) return;
		FMOD::Studio::EventInstance* event_inst = nullptr;
		FMOD_RESULT result = event_desc->createInstance(&event_inst);
		if (result != FMOD_OK)
		{
			console::log_error("Failed to create audio event instance: " + path);
			return;
		}
		event_inst->start();
		event_inst->release();
	}

	void stop(const std::string& path)
	{
		FMOD::Studio::EventDescription* event_desc = nullptr;
		if (!_get_event_description(path, &event_desc)) return;
		for (FMOD::Studio::EventInstance* event_inst : _get_event_instances(event_desc))
			event_inst->stop(FMOD_STUDIO_STOP_IMMEDIATE);
	}

	void stop_all()
	{
		FMOD::Studio::Bus* master_bus = nullptr;
		FMOD_RESULT result = _system->getBus("bus:/", &master_bus);
		assert(result == FMOD_OK);
		master_bus->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE);
	}
}

