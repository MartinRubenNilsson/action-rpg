#pragma once

namespace console
{
	void startup();
	void update(); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
	void toggle_visible();
	void clear();
	void log(const std::string& message);
	void execute(const std::string& command_line);
}
