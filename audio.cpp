#include "stdafx.h"
#include "audio.h"
#include <fmod/fmod_studio.hpp>
#include "pool.h"
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
	const std::string BUS_MASTER = "bus:/";
	const std::string BUS_SOUND = "bus:/sound";
	const std::string BUS_MUSIC = "bus:/music";
	const float _PIXELS_PER_FMOD_UNIT = 16.f;

	bool log_errors =
#ifdef _DEBUG
		true;
#else
		false;
#endif

	struct Event
	{
		FMOD::Studio::EventInstance* instance = nullptr;
	};

	FMOD::Studio::System* _system = nullptr;
	FMOD::Studio::EventInstance* _event_buffer[1024] = {};
	Pool<Event> _event_pool;
	std::unordered_set<std::string> _events_played_this_frame;

	void* _handle_to_userdata(Handle<Event> handle)
	{
		uint32_t uint = *(uint32_t*)&handle;
		return (void*)(uintptr_t)uint;
	}

	Handle<Event> _userdata_to_handle(void* userdata)
	{
		uint32_t uint = (uint32_t)(uintptr_t)userdata;
		return *(Handle<Event>*)&uint;
	}

	FMOD_RESULT F_CALLBACK _on_event_destroyed(
		FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
		FMOD_STUDIO_EVENTINSTANCE* instance,
		void* parameters)
	{
		void* userdata = nullptr;
		((FMOD::Studio::EventInstance*)instance)->getUserData((void**)&userdata);
		if (!userdata) return FMOD_OK;
		_event_pool.free(_userdata_to_handle(userdata));
		return FMOD_OK;
	}

	FMOD::Studio::Bus* _get_bus(const std::string& path)
	{
		FMOD::Studio::Bus* bus = nullptr;
		if (_system->getBus(path.c_str(), &bus) != FMOD_OK && log_errors) {
			console::log_error("Could not find audio bus: " + path);
		}
		return bus;
	}

	FMOD::Studio::EventDescription* _get_event_description(const char* path)
	{
		FMOD::Studio::EventDescription* desc = nullptr;
		if (_system->getEvent(path, &desc) != FMOD_OK && log_errors) {
			console::log_error("Could not find audio event: " + std::string(path));
		}
		return desc;
	}

	FMOD::Studio::EventInstance* _create_event_instance(FMOD::Studio::EventDescription* desc)
	{
		FMOD::Studio::EventInstance* instance = nullptr;
		if (desc->createInstance(&instance) != FMOD_OK && log_errors) {
			console::log_error("Failed to create audio event");
		}
		return instance;
	}

	// The returned span is valid only until the next call to _get_event_instances.
	std::span<FMOD::Studio::EventInstance*> _get_event_instances(FMOD::Studio::EventDescription* desc)
	{
		int count = 0;
		desc->getInstanceList(_event_buffer, _countof(_event_buffer), &count);
		return std::span(_event_buffer, count);
	}

	FMOD::Studio::EventInstance* _get_event_instance(Handle<Event> handle)
	{
		Event* ev = _event_pool.get(handle);
		return ev ? ev->instance : nullptr;
	}

	void initialize()
	{
		FMOD_RESULT result = FMOD::Studio::System::create(&_system);
		assert(result == FMOD_OK);
		FMOD_STUDIO_INITFLAGS flags = FMOD_STUDIO_INIT_LIVEUPDATE;
#ifdef _DEBUG
		flags |= FMOD_STUDIO_INIT_LIVEUPDATE;
#endif
		result = _system->initialize(
			512, // max channels
			flags,
			FMOD_INIT_NORMAL,
			nullptr);
		assert(result == FMOD_OK);
	}

	void shutdown()
	{
		_system->release();
		_system = nullptr;
	}

	void update()
	{
		_system->update();
		_events_played_this_frame.clear();
	}

	void load_bank_from_file(const std::string& path)
	{
		FMOD::Studio::Bank* bank = nullptr;
		FMOD_RESULT result = _system->loadBankFile(path.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
		if (result != FMOD_OK && log_errors) {
			console::log_error("Failed to load audio bank: " + path);
		}
	}

	FMOD_3D_ATTRIBUTES _pos_to_3d_attributes(const Vector2f& position)
	{
		FMOD_3D_ATTRIBUTES attributes{};
		attributes.position = { position.x / _PIXELS_PER_FMOD_UNIT, -position.y / _PIXELS_PER_FMOD_UNIT, 0.f };
		attributes.forward = { 0.f, 0.f, 1.f };
		attributes.up = { 0.f, 1.f, 0.f };
		return attributes;
	}

	Vector2f _3d_attributes_to_pos(const FMOD_3D_ATTRIBUTES& attributes)
	{
		return { attributes.position.x * _PIXELS_PER_FMOD_UNIT, -attributes.position.y * _PIXELS_PER_FMOD_UNIT };
	}

	void set_listener_position(const Vector2f& position)
	{
		FMOD_3D_ATTRIBUTES attributes = _pos_to_3d_attributes(position);
		_system->setListenerAttributes(0, &attributes);
	}

	Vector2f get_listener_position()
	{
		FMOD_3D_ATTRIBUTES attributes{};
		_system->getListenerAttributes(0, &attributes);
		return _3d_attributes_to_pos(attributes);
	}

	bool set_parameter(const std::string& name, float value)
	{
		FMOD_RESULT result = _system->setParameterByName(name.c_str(), value);
		if (result == FMOD_OK) return true;
		if (log_errors) {
			console::log_error("Could not find audio parameter: " + name + "=" + std::to_string(value));
		}
		return false;
	}

	bool get_parameter(const std::string& name, float& value)
	{
		FMOD_RESULT result = _system->getParameterByName(name.c_str(), &value);
		if (result == FMOD_OK) return true;
		if (log_errors) {
			console::log_error("Could not find audio parameter: " + name);
		}
		return false;
	}

	bool set_parameter_label(const std::string& name, const std::string& label)
	{
		FMOD_RESULT result = _system->setParameterByNameWithLabel(name.c_str(), label.c_str());
		if (result == FMOD_OK) return true;
		if (log_errors) {
			console::log_error("Could not find audio parameter label: " + name + "=" + label);
		}
		return false;
	}

	bool get_parameter_label(const std::string& name, std::string& label)
	{
		float value = 0.f;
		FMOD_RESULT result = _system->getParameterByName(name.c_str(), &value);
		if (result != FMOD_OK) {
			if (log_errors) {
				console::log_error("Could not find audio parameter: " + name);
			}
			return false;
		}
		char label_buffer[256];
		result = _system->getParameterLabelByName(
			name.c_str(), (int)value, label_buffer, _countof(label_buffer), nullptr);
		if (result != FMOD_OK) {
			if (log_errors) {
				console::log_error("Could not get parameter label: " + name + "=" + std::to_string(value));
			}
		}
		label = label_buffer;
		return true;
	}

	bool is_valid(const std::string& event_path)
	{
		FMOD::Studio::EventDescription* desc_dummy = nullptr;
		return _system->getEvent(event_path.c_str(), &desc_dummy) == FMOD_OK;
	}

	bool is_any_playing(const std::string& event_path)
	{
		FMOD::Studio::EventDescription* desc = _get_event_description(event_path.c_str());
		if (!desc) return false;
		for (FMOD::Studio::EventInstance* instance : _get_event_instances(desc)) {
			FMOD_STUDIO_PLAYBACK_STATE state;
			instance->getPlaybackState(&state);
			if (state == FMOD_STUDIO_PLAYBACK_PLAYING) return true;
		}
		return false;
	}

	Handle<Event> create_event(const EventDesc& desc)
	{
		if (!desc.path) return Handle<Event>();
		if (_events_played_this_frame.contains(desc.path)) return Handle<Event>();
		FMOD::Studio::EventDescription* studio_desc = _get_event_description(desc.path);
		if (!studio_desc) return Handle<Event>();
		FMOD::Studio::EventInstance* instance = _create_event_instance(studio_desc);
		if (!instance) return Handle<Event>();
		Handle<Event> handle = _event_pool.emplace(Event{ .instance = instance });
		_events_played_this_frame.insert(desc.path);
		instance->setCallback(_on_event_destroyed, FMOD_STUDIO_EVENT_CALLBACK_DESTROYED);
		instance->setUserData(_handle_to_userdata(handle));
		instance->setVolume(desc.volume);
		FMOD_3D_ATTRIBUTES attributes = _pos_to_3d_attributes(desc.position);
		instance->set3DAttributes(&attributes);
		if (desc.start) {
			instance->start();
		}
		if (desc.release) {
			instance->release();
		}
		return handle;
	}

	bool stop_event(Handle<Event> handle)
	{
		FMOD::Studio::EventInstance* instance = _get_event_instance(handle);
		if (!instance) return false;
		instance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
		return true;
	}

	bool set_event_volume(Handle<Event> handle, float volume)
	{
		FMOD::Studio::EventInstance* instance = _get_event_instance(handle);
		if (!instance) return false;
		instance->setVolume(volume);
		return true;
	}

	bool get_event_volume(Handle<Event> handle, float& volume)
	{
		FMOD::Studio::EventInstance* instance = _get_event_instance(handle);
		if (!instance) return false;
		instance->getVolume(&volume);
		return true;
	}

	bool set_event_position(Handle<Event> handle, const Vector2f& position)
	{
		FMOD::Studio::EventInstance* instance = _get_event_instance(handle);
		if (!instance) return false;
		FMOD_3D_ATTRIBUTES attributes = _pos_to_3d_attributes(position);
		instance->set3DAttributes(&attributes);
		return true;
	}

	bool get_event_position(Handle<Event> handle, Vector2f& position)
	{
		FMOD::Studio::EventInstance* instance = _get_event_instance(handle);
		if (!instance) return false;
		FMOD_3D_ATTRIBUTES attributes{};
		instance->get3DAttributes(&attributes);
		position = _3d_attributes_to_pos(attributes);
		return true;
	}

	bool set_bus_volume(const std::string& bus_path, float volume)
	{
		FMOD::Studio::Bus* bus = _get_bus(bus_path);
		if (!bus) return false;
		bus->setVolume(volume);
		return true;
	}

	bool get_bus_volume(const std::string& bus_path, float& volume)
	{
		FMOD::Studio::Bus* bus = _get_bus(bus_path);
		if (!bus) return false;
		bus->getVolume(&volume);
		return true;
	}

	bool stop_all_in_bus(const std::string& bus_path)
	{
		FMOD::Studio::Bus* bus = _get_bus(bus_path);
		if (!bus) return false;
		bus->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE);
		return true;
	}
}

