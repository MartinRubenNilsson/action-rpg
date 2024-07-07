#pragma once

namespace audio
{
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

	bool is_valid(const std::string& event_path);
	bool is_any_playing(const std::string& event_path);

	struct EventOptions
	{
		float volume = 1.f;
		Vector2f position;
		bool start = true;
		bool release = true;
	};

	// Returns EventHandle::Invalid on failure.
	EventHandle play(const std::string& event_path, const EventOptions& options = EventOptions());
	bool stop(EventHandle handle);
	bool set_volume(EventHandle handle, float volume);
	bool get_volume(EventHandle handle, float& volume);
	bool set_position(EventHandle handle, const Vector2f& position);
	bool get_position(EventHandle handle, Vector2f& position);

	// BUSES

	bool set_bus_volume(const std::string& bus_path, float volume);
	bool get_bus_volume(const std::string& bus_path, float& volume);
	bool stop_all_in_bus(const std::string& bus_path = BUS_MASTER);
}

