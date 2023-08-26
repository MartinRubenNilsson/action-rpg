#pragma once

namespace console
{
	void initialize(sf::RenderWindow& window);
	void update(); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.

	void toggle_visible();
	void clear();
	void log(const std::string& message);
	void log_error(const std::string& message);
	void execute(const std::string& command_line);

	// Scripts are kept in the assets/scripts folder.
	// The script name should not include the .script extension.
	void execute_script(const std::string& script_name);
}
