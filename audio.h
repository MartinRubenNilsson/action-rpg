#pragma once

namespace audio
{
	extern const int INVALID_EVENT_ID;
	extern const std::string BUS_MASTER;
	extern const std::string BUS_SOUND;
	extern const std::string BUS_MUSIC;

	extern bool log_errors;

	void initialize();
	void shutdown();
	void load_bank_files(const std::filesystem::path& dir);
	void update();

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

	// Returns a new unique event id on success, or INVALID_EVENT_ID on failure.
	int play(const std::string& event_path, const EventOptions& options = EventOptions());
	bool is_valid(int event_id);
	bool stop(int event_id);
	bool set_volume(int event_id, float volume);
	bool get_volume(int event_id, float& volume);
	bool set_position(int event_id, const Vector2f& position);
	bool get_position(int event_id, Vector2f& position);

	// BUSES

	bool set_bus_volume(const std::string& bus_path, float volume);
	bool get_bus_volume(const std::string& bus_path, float& volume);
	bool stop_all_in_bus(const std::string& bus_path = BUS_MASTER);
}

