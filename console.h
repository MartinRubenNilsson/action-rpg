#pragma once

namespace console
{
	void initialize();
	void update(float dt); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
	void process_event(const sf::Event& event);

	bool is_showing();
	void show(bool show);
	void toggle_show();
	void clear();
	void sleep(float seconds);
	void log(const std::string& message);
	void log_error(const std::string& message, bool show_console = true);
	void execute(const std::string& command_line, bool defer = false);
	// Scripts are kept in the assets/scripts folder.
	// The script name should not include the .script extension.
	void execute_script(const std::string& script_name);
	void bind(sf::Keyboard::Key key, const std::string& command_line);
	void bind(const std::string& key_string, const std::string& command_line);
	void unbind(sf::Keyboard::Key key);
	void unbind(const std::string& key_string);
	void write_help_file(const std::string& filename);
}
