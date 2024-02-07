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
	const std::string BUS_SOUND = "bus:/sound";
	const std::string BUS_MUSIC = "bus:/music";
	const float _PIXELS_PER_METER = 16.f;
	FMOD::Studio::System* _system = nullptr;
	FMOD::Studio::EventInstance* _event_buffer[1024] = {};

	FMOD::Studio::Bus* _get_bus(const std::string& path)
	{
		FMOD::Studio::Bus* bus = nullptr;
		if (_system->getBus(path.c_str(), &bus) != FMOD_OK && log_errors)
			console::log_error("Could not find audio bus: " + path);
		return bus;
	}

	FMOD::Studio::EventDescription* _get_event_description(const std::string& path)
	{
		FMOD::Studio::EventDescription* desc = nullptr;
		if (_system->getEvent(path.c_str(), &desc) != FMOD_OK && log_errors)
			console::log_error("Could not find audio event: " + path);
		return desc;
	}

	FMOD::Studio::EventInstance* _create_event_instance(FMOD::Studio::EventDescription* desc)
	{
		FMOD::Studio::EventInstance* instance = nullptr;
		if (desc->createInstance(&instance) != FMOD_OK && log_errors)
			console::log_error("Failed to create audio event");
		return instance;
	}

	// The returned span is valid only until the next call to _get_event_instances.
	std::span<FMOD::Studio::EventInstance*> _get_event_instances(FMOD::Studio::EventDescription* desc)
	{
		int count = 0;
		desc->getInstanceList(_event_buffer, _countof(_event_buffer), &count);
		return std::span(_event_buffer, count);
	}

	void initialize()
	{
		FMOD_RESULT result = FMOD::Studio::System::create(&_system);
		assert(result == FMOD_OK);
		result = _system->initialize(
			512, // max channels
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

	void load_bank_files(const std::filesystem::path& dir)
	{
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator(dir)) {
			if (!entry.is_regular_file()) continue;
			if (entry.path().extension() != ".bank") continue;
			FMOD::Studio::Bank* bank = nullptr;
			FMOD_RESULT result = _system->loadBankFile(
				entry.path().string().c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
			if (result != FMOD_OK && log_errors)
				console::log_error("Failed to load audio bank: " + entry.path().string());
		}
	}

	void update() {
		_system->update();
	}

	FMOD_3D_ATTRIBUTES _pos_to_3d_attribs(const sf::Vector2f& position)
	{
		FMOD_3D_ATTRIBUTES attributes{};
		attributes.position = { position.x / _PIXELS_PER_METER, -position.y / _PIXELS_PER_METER, 0.f };
		attributes.forward = { 0.f, 0.f, 1.f };
		attributes.up = { 0.f, 1.f, 0.f };
		return attributes;
	}

	sf::Vector2f _3d_attribs_to_pos(const FMOD_3D_ATTRIBUTES& attributes) {
		return { attributes.position.x * _PIXELS_PER_METER, -attributes.position.y * _PIXELS_PER_METER };
	}

	void set_listener_position(const sf::Vector2f& position)
	{
		FMOD_3D_ATTRIBUTES attributes = _pos_to_3d_attribs(position);
		_system->setListenerAttributes(0, &attributes);
	}

	sf::Vector2f get_listener_position()
	{
		FMOD_3D_ATTRIBUTES attributes{};
		_system->getListenerAttributes(0, &attributes);
		return _3d_attribs_to_pos(attributes);
	}

	bool set_parameter(const std::string& name, float value)
	{
		FMOD_RESULT result = _system->setParameterByName(name.c_str(), value);
		if (result != FMOD_OK) {
			if (log_errors)
				console::log_error("Could not find audio parameter: " + name + "=" + std::to_string(value));
			return false;
		}
		return true;
	}

	bool get_parameter(const std::string& name, float& value)
	{
		FMOD_RESULT result = _system->getParameterByName(name.c_str(), &value);
		if (result != FMOD_OK) {
			if (log_errors)
				console::log_error("Could not find audio parameter: " + name);
			return false;
		}
		return true;
	}

	bool set_parameter_label(const std::string& name, const std::string& label)
	{
		FMOD_RESULT result = _system->setParameterByNameWithLabel(name.c_str(), label.c_str());
		if (result != FMOD_OK) {
			if (log_errors)
				console::log_error("Could not find audio parameter label: " + name + "=" + label);
			return false;
		}
		return true;
	}

	bool is_valid(const std::string& event_path)
	{
		FMOD::Studio::EventDescription* desc_dummy = nullptr;
		return _system->getEvent(event_path.c_str(), &desc_dummy) == FMOD_OK;
	}

	bool is_any_playing(const std::string& event_path)
	{
		FMOD::Studio::EventDescription* desc = _get_event_description(event_path);
		if (!desc) return false;
		for (FMOD::Studio::EventInstance* instance : _get_event_instances(desc)) {
			FMOD_STUDIO_PLAYBACK_STATE state;
			instance->getPlaybackState(&state);
			if (state == FMOD_STUDIO_PLAYBACK_PLAYING)
				return true;
		}
		return false;
	}

	bool play(const std::string& event_path, entt::entity entity)
	{
		FMOD::Studio::EventDescription* desc = _get_event_description(event_path);
		if (!desc) return false;
		FMOD::Studio::EventInstance* instance = _create_event_instance(desc);
		if (!instance) return false;
		instance->setUserData((void*)entity);
		instance->start();
		instance->release();
		return true;
	}

	bool play_at_position(const std::string& event_path, const sf::Vector2f& position)
	{
		FMOD::Studio::EventDescription* desc = _get_event_description(event_path);
		if (!desc) return false;
		FMOD::Studio::EventInstance* instance = _create_event_instance(desc);
		if (!instance) return false;
		FMOD_3D_ATTRIBUTES attributes = _pos_to_3d_attribs(position);
		instance->set3DAttributes(&attributes);
		instance->start();
		instance->release();
		return true;
	}

	entt::entity _get_entity(FMOD::Studio::EventInstance* instance)
	{
		entt::entity entity = entt::null;
		instance->getUserData((void**)&entity);
		return entity;
	}

	bool stop(const std::string& event_path, entt::entity entity)
	{
		FMOD::Studio::EventDescription* desc = _get_event_description(event_path);
		if (!desc) return false;
		for (FMOD::Studio::EventInstance* instance : _get_event_instances(desc)) {
			if (entity == _get_entity(instance)) {
				instance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
				return true;
			}
		}
		return false;
	}

	void set_bus_volume(const std::string& bus_path, float volume)
	{
		if (FMOD::Studio::Bus* bus = _get_bus(bus_path))
			bus->setVolume(volume);
	}

	float get_bus_volume(const std::string& bus_path)
	{
		float volume = 0.f;
		if (FMOD::Studio::Bus* bus = _get_bus(bus_path))
			bus->getVolume(&volume);
		return volume;
	}

	void stop_all_in_bus(const std::string& bus_path)
	{
		if (FMOD::Studio::Bus* bus = _get_bus(bus_path))
			bus->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE);
	}
}

