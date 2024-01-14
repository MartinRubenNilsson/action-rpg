#pragma once

namespace audio
{
	extern bool log_errors;
	extern const std::string BUS_MASTER;

	void initialize();
	void shutdown();
	void load_bank_files(const std::filesystem::path& dir);
	void update();

	bool set_parameter(const std::string& name, float value);
	bool get_parameter(const std::string& name, float& value);
	bool set_parameter_label(const std::string& name, const std::string& label);
	//bool get_parameter_label(const std::string& name, std::string& label); // TODO

	bool is_valid(const std::string& event_path);
	bool is_playing(const std::string& event_path);
	// Creates and starts a new event instance with the given path.
	// If the event is not one-shot, it may keep playing indefinitely.
	bool play(const std::string& event_path);
	void stop_all(const std::string& event_path);
	bool stop_all_in_bus(const std::string& bus_path = BUS_MASTER);
}

