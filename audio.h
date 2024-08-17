#pragma once

namespace audio
{
	struct Event;

	extern const std::string BUS_MASTER;
	extern const std::string BUS_SOUND;
	extern const std::string BUS_MUSIC;

	extern bool log_errors;

	void initialize();
	void shutdown();
	void update();
	void load_bank_from_file(const std::string& path);

	// LISTENERS

	void set_listener_position(const Vector2f& position);
	Vector2f get_listener_position();

	// GLOBAL PARAMETERS

	bool set_parameter(const std::string& name, float value);
	bool get_parameter(const std::string& name, float& value);
	bool set_parameter_label(const std::string& name, const std::string& label);
	bool get_parameter_label(const std::string& name, std::string& label);

	// EVENTS

	bool is_any_playing(const char* path);

	struct EventDesc
	{
		const char* path = nullptr;
		float volume = 1.f;
		Vector2f position;
		bool start = true;
		bool release = true;
	};

	Handle<Event> create_event(const EventDesc&& desc);
	bool stop_event(Handle<Event> handle);
	bool set_event_volume(Handle<Event> handle, float volume);
	bool get_event_volume(Handle<Event> handle, float& volume);
	bool set_event_position(Handle<Event> handle, const Vector2f& position);
	bool get_event_position(Handle<Event> handle, Vector2f& position);

	// BUSES

	bool set_bus_volume(const std::string& bus_path, float volume);
	bool get_bus_volume(const std::string& bus_path, float& volume);
	bool stop_all_in_bus(const std::string& bus_path = BUS_MASTER);
}

