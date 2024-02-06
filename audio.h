#pragma once

namespace audio
{
	extern bool log_errors;
	extern const std::string BUS_MASTER;
	extern const std::string BUS_SOUND;
	extern const std::string BUS_MUSIC;

	void initialize();
	void shutdown();
	void load_bank_files(const std::filesystem::path& dir);
	void update();

	void set_listener_position(const sf::Vector2f& position);
	sf::Vector2f get_listener_position();

	bool set_parameter(const std::string& name, float value);
	bool get_parameter(const std::string& name, float& value);
	bool set_parameter_label(const std::string& name, const std::string& label);
	//bool get_parameter_label(const std::string& name, std::string& label); // TODO

	bool is_valid(const std::string& event_path);
	bool is_any_playing(const std::string& event_path);
	bool play(const std::string& event_path, entt::entity entity = entt::null);
	bool play_at_position(const std::string& event_path, const sf::Vector2f& position);
	bool stop(const std::string& event_path, entt::entity entity = entt::null);

	void set_bus_volume(const std::string& bus_path, float volume);
	float get_bus_volume(const std::string& bus_path);
	void stop_all_in_bus(const std::string& bus_path = BUS_MASTER);
}

