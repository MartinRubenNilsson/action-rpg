#include "console.h"
#include <imgui.h>
#include <imgui_stdlib.h>

namespace console
{
	bool _visible = false;
	std::vector<std::string> _lines;
	std::string _input;

	void update()
	{
        if (!_visible)
			return;

		if (ImGui::Begin("Console"))
		{
			// LOG

			// Height of 1 separator + 1 input text
			float input_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
			int window_flags = ImGuiWindowFlags_HorizontalScrollbar;
			if (ImGui::BeginChild("Log", ImVec2(0, -input_height), false, window_flags))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
				for (const auto& line : _lines)
					ImGui::TextUnformatted(line.c_str());
				ImGui::PopStyleVar();
			}
			ImGui::EndChild();

			ImGui::Separator();

            // INPUT

			ImGui::PushItemWidth(-1); // Use all available width
			ImGui::SetKeyboardFocusHere();
			if (ImGui::InputText("##Input", &_input, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				_lines.push_back(_input);
				_input.clear();
			}
			ImGui::PopItemWidth();
		}
        ImGui::End();
	}

	void toggle_visible()
	{
		_visible = !_visible;
	}
}
