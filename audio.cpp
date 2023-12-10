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
	bool log_errors =
#ifdef _DEBUG
		true;
#else
		false;
#endif
	const std::string BUS_MASTER = "bus:/";
	const int _MAX_AUDIO_CHANNELS = 512;
	const int _EVENT_BUFFER_CAPACITY = 1024;
	FMOD::Studio::System* _system = nullptr;
	FMOD::Studio::EventInstance* _event_buffer[_EVENT_BUFFER_CAPACITY] = {};

	FMOD::Studio::Bus* _get_bus(const std::string& path)
	{
		FMOD::Studio::Bus* bus = nullptr;
		FMOD_RESULT result = _system->getBus(path.c_str(), &bus);
		if (result != FMOD_OK) {
			if (log_errors)
				console::log_error("Failed to get audio bus: " + path);
			return nullptr;
		}
		return bus;
	}

	FMOD::Studio::EventDescription* _get_event_description(const std::string& path)
	{
		FMOD::Studio::EventDescription* desc = nullptr;
		FMOD_RESULT result = _system->getEvent(path.c_str(), &desc);
		if (result != FMOD_OK) {
			if (log_errors)
				console::log_error("Failed to get audio event description: " + path);
			return nullptr;
		}
		return desc;
	}

	FMOD::Studio::EventInstance* _create_event_instance(FMOD::Studio::EventDescription* desc)
	{
		FMOD::Studio::EventInstance* instance = nullptr;
		FMOD_RESULT result = desc->createInstance(&instance);
		if (result != FMOD_OK) {
			if (log_errors)
				console::log_error("Failed to create audio event instance");
			return nullptr;
		}
		return instance;
	}

	// Returns the number of event instances written to the buffer.
	int _get_event_instances(FMOD::Studio::EventDescription* desc)
	{
		int count = 0;
		desc->getInstanceList(_event_buffer, _EVENT_BUFFER_CAPACITY, &count);
		return count;
	}

	void initialize()
	{
		FMOD_RESULT result = FMOD::Studio::System::create(&_system);
		assert(result == FMOD_OK);
		result = _system->initialize(
			_MAX_AUDIO_CHANNELS,
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

	void load_assets(const std::filesystem::path& dir)
	{
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator(dir))
		{
			if (!entry.is_regular_file()) continue;
			if (entry.path().extension() != ".bank") continue;
			FMOD::Studio::Bank* bank = nullptr;
			FMOD_RESULT result = _system->loadBankFile(
				entry.path().string().c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
			if (result != FMOD_OK) {
				if (log_errors)
					console::log_error("Failed to load audio bank: " + entry.path().string());
			}
		}
	}

	bool set_parameter(const std::string& name, float value)
	{
		FMOD_RESULT result = _system->setParameterByName(name.c_str(), value);
		if (result != FMOD_OK) {
			if (log_errors)
				console::log_error("Failed to set audio parameter: " + name + "=" + std::to_string(value));
			return false;
		}
		return true;
	}

	bool get_parameter(const std::string& name, float& value)
	{
		FMOD_RESULT result = _system->getParameterByName(name.c_str(), &value);
		if (result != FMOD_OK) {
			if (log_errors)
				console::log_error("Failed to get audio parameter: " + name);
			return false;
		}
		return true;
	}

	bool set_parameter_label(const std::string& name, const std::string& label)
	{
		FMOD_RESULT result = _system->setParameterByNameWithLabel(name.c_str(), label.c_str());
		if (result != FMOD_OK) {
			if (log_errors)
				console::log_error("Failed to set audio parameter label: " + name + "=" + label);
			return false;
		}
		return true;
	}

	bool is_valid(const std::string& path)
	{
		FMOD::Studio::EventDescription* desc_dummy = nullptr;
		return _system->getEvent(path.c_str(), &desc_dummy) == FMOD_OK;
	}

	bool is_playing(const std::string& path)
	{
		auto desc = _get_event_description(path);
		if (!desc) return false;
		int count = _get_event_instances(desc);
		for (int i = 0; i < count; ++i) {
			FMOD_STUDIO_PLAYBACK_STATE state;
			_event_buffer[i]->getPlaybackState(&state);
			if (state == FMOD_STUDIO_PLAYBACK_PLAYING)
				return true;
		}
		return false;
	}

	bool play(const std::string& path)
	{
		auto desc = _get_event_description(path);
		if (!desc) return false;
		auto instance = _create_event_instance(desc);
		if (!instance) return false;
		instance->start();
		instance->release();
		return true;
	}

	void stop(const std::string& path)
	{
		auto desc = _get_event_description(path);
		if (!desc) return;
		int count = _get_event_instances(desc);
		for (int i = 0; i < count; ++i)
			_event_buffer[i]->stop(FMOD_STUDIO_STOP_IMMEDIATE);
	}

	bool stop_all(const std::string& path)
	{
		auto bus = _get_bus(path);
		if (!bus) return false;
		bus->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE);
		return true;
	}
}

