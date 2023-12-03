#pragma once

namespace audio
{
	extern bool log_errors;
	extern const std::string BUS_MASTER;

	void initialize();
	void shutdown();
	void update();

	// Loads all FMOD banks (.bank) in the given directory.
	void load_assets(const std::filesystem::path& dir);

	bool set_parameter(const std::string& name, float value);
	bool get_parameter(const std::string& name, float& value);
	bool set_parameter_label(const std::string& name, const std::string& label);
	//bool get_parameter_label(const std::string& name, std::string& label); // TODO

	// Checks if an event with the given path exists in the loaded banks.
	bool exists(const std::string& path);

	// Checks if an event instance with the given path exists and is currently playing.
	bool is_playing(const std::string& path);

	// Creates and starts a new event instance with the given path.
	// If the event is not one-shot, it may keep playing indefinitely.
	bool play(const std::string& path);

	// Stops all event instances with the given path.
	void stop(const std::string& path);

	// Stops all event instances in the given bus.
	bool stop_all(const std::string& path = BUS_MASTER);
}

