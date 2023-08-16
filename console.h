#pragma once

namespace console
{
	// Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
	void update();
	void toggle_visible();
}
