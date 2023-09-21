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

#define AUDIO_MAX_CHANNELS 512

namespace audio
{
	FMOD::Studio::System* _system = nullptr;

	void initialize()
	{
		FMOD::Studio::System::create(&_system);
		_system->initialize(
			AUDIO_MAX_CHANNELS,
			FMOD_STUDIO_INIT_NORMAL,
			FMOD_INIT_NORMAL,
			nullptr);
	}

	void shutdown() {
		_system->release();
	}

	void update() {
		_system->update();
	}

	void load_banks()
	{
		for (const auto& entry : std::filesystem::directory_iterator("assets/audio/banks"))
		{
			if (entry.path().extension() != ".bank")
				continue;
			FMOD::Studio::Bank *bank = nullptr;
			FMOD_RESULT result = _system->loadBankFile(
				entry.path().string().c_str(),
				FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
			if (result != FMOD_OK)
				console::log_error("Failed to load audio bank: " + entry.path().string());
		}
	}

	void play(const std::string& path)
	{
		FMOD::Studio::EventDescription* event_desc = nullptr;
		FMOD_RESULT result = _system->getEvent(path.c_str(), &event_desc);
		if (result != FMOD_OK)
		{
			console::log_error("Failed to get audio event description: " + path);
			return;
		}

		FMOD::Studio::EventInstance* event_instance = nullptr;
		result = event_desc->createInstance(&event_instance);
		if (result != FMOD_OK)
		{
			console::log_error("Failed to create audio event instance: " + path);
			return;
		}

		event_instance->start();
		event_instance->release();
	}

	void stop_all()
	{
		FMOD::Studio::Bus* master_bus = nullptr;
		FMOD_RESULT result = _system->getBus("bus:/", &master_bus);
		if (result != FMOD_OK)
		{
			console::log_error("Failed to get audio master bus");
			return;
		}
		master_bus->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE);
	}
}

