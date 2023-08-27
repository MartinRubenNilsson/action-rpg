#pragma once

namespace console
{
	void initialize(sf::RenderWindow& window);
	void update(); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
	void process_event(const sf::Event& event);

	void toggle_visible();
	void clear();
	void log(const std::string& message);
	void log_error(const std::string& message, bool make_visible = true);
	void execute(const std::string& command_line);

	// Scripts are kept in the assets/scripts folder.
	// The script name should not include the .script extension.
	void execute_script(const std::string& script_name);

	void bind(sf::Keyboard::Key key, const std::string& command_line);
	void bind(const std::string& key_string, const std::string& command_line);
	void unbind(sf::Keyboard::Key key);
	void unbind(const std::string& key_string);
}
