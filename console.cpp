#include "console.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <CLI/CLI.hpp>

namespace console
{
	// Solarized color palette

	const ImColor _base03  = IM_COL32(  0,  43,  54, 255);
	const ImColor _base02  = IM_COL32(  7,  54,  66, 255);
	const ImColor _base01  = IM_COL32( 88, 110, 117, 255);
	const ImColor _base00  = IM_COL32(101, 123, 131, 255);
	const ImColor _base0   = IM_COL32(131, 148, 150, 255);
	const ImColor _base1   = IM_COL32(147, 161, 161, 255);
	const ImColor _base2   = IM_COL32(238, 232, 213, 255);
	const ImColor _base3   = IM_COL32(253, 246, 227, 255);
	const ImColor _yellow  = IM_COL32(181, 137,   0, 255);
	const ImColor _orange  = IM_COL32(203,  75,  22, 255);
	const ImColor _red     = IM_COL32(220,  50,  47, 255);
	const ImColor _magenta = IM_COL32(211,  54, 130, 255);
	const ImColor _violet  = IM_COL32(108, 113, 196, 255);
	const ImColor _blue    = IM_COL32( 38, 139, 210, 255);
	const ImColor _cyan    = IM_COL32( 42, 161, 152, 255);
	const ImColor _green   = IM_COL32(133, 153,   0, 255);

	CLI::App _app("Console", "");
	std::string _command_line;
	std::vector<std::string> _command_history;
	std::vector<std::string>::iterator _command_history_it = _command_history.end();
	std::vector<std::pair<std::string, ImColor>> _history;
	bool _visible = false;
	bool _reclaim_focus = false;

	// Handles command history navigation
	int _input_text_callback(ImGuiInputTextCallbackData* data)
	{
		if (_command_history.empty()) return 0;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (_command_history_it > _command_history.begin())
			{
				_command_history_it--;
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, _command_history_it->c_str());
			}
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (_command_history_it < _command_history.end() - 1)
			{
				_command_history_it++;
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, _command_history_it->c_str());
			}
		}
		return 0;
	}

	// Defined in console_commands.cpp
	extern void _setup_commands(CLI::App& app);

	void startup()
	{
		// SETUP IMGUI STYLE
		{
			// https://github.com/ocornut/imgui/issues/707#issuecomment-252413954

			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 5.3f;
			style.FrameRounding = 2.3f;
			style.ScrollbarRounding = 0;
			
			style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
			style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
			style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
			style.Colors[ImGuiCol_ChildBg]				 = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
			style.Colors[ImGuiCol_Border]                = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
			style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
			style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
			style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
			style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
			style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
			style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
			style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
			style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
			style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
			style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
			style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
			style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
			style.Colors[ImGuiCol_Button]                = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
			style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
			style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
			style.Colors[ImGuiCol_Header]                = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
			style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
			style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
			style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
			style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
			style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
			style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
			style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
		}

		_setup_commands(_app);

		log("Type -h or --help for a list of commands");
	}

	void update()
	{
        if (!_visible)
			return;
		
		ImGui::SetNextWindowBgAlpha(0.35f);
		if (ImGui::Begin("Console", &_visible))
		{
			ImGui::SetWindowFontScale(2.f);

			// HISTORY
			{
				float reserved_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
				if (ImGui::BeginChild(
					"History",
					ImVec2(0, -reserved_height), // Leave room for 1 separator + 1 input text
					false,
					ImGuiWindowFlags_HorizontalScrollbar))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
					for (const auto& [line, color] : _history)
						ImGui::TextColored(color, line.c_str());
					ImGui::PopStyleVar();
					if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
						ImGui::SetScrollHereY(1.0f); // Scroll to bottom
				}
				ImGui::EndChild();
			}

			ImGui::Separator();

            // COMMAND LINE

			ImGui::PushItemWidth(-1); // Use all available width
			if (ImGui::InputText(
				"##CommandLine",
				&_command_line,
				ImGuiInputTextFlags_EnterReturnsTrue |
				ImGuiInputTextFlags_CallbackHistory |
				ImGuiInputTextFlags_EscapeClearsAll,
				_input_text_callback))
			{
				execute(_command_line);
				_command_line.clear();
				_reclaim_focus = true;
			}
			ImGui::PopItemWidth();

			// AUTO FOCUS

			ImGui::SetItemDefaultFocus();
			if (_reclaim_focus)
				ImGui::SetKeyboardFocusHere(-1); // Auto focus command line
			_reclaim_focus = false;
		}
        ImGui::End();
	}

	void toggle_visible()
	{
		_visible = !_visible;
		if (_visible)
			_reclaim_focus = true;
	}

	void clear()
	{
		_history.clear();
	}

	void log(const std::string& message)
	{
		_history.emplace_back(message, ImGui::GetStyle().Colors[ImGuiCol_Text]);
	}

	void log_error(const std::string& message)
	{
		_history.emplace_back(message, _red);
	}

	void execute(const std::string& command_line)
	{
		_command_history.push_back(command_line);
		_command_history_it = _command_history.end();
		_history.emplace_back(command_line, _blue);

		try
		{
			_app.parse(command_line); // throws
		}
		catch (const CLI::CallForHelp&)
		{
			log(_app.help());
		}
		catch (const CLI::ParseError& parse_error)
		{
			_history.emplace_back(parse_error.what(), _yellow);
		}
	}
}
