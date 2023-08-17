#include "console.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <CLI/CLI.hpp>
#include "map.h"

extern void close_window();

namespace console
{
	CLI::App _app("Console", "");

	bool _visible = false;
	bool _reclaim_focus = false;
	std::vector<std::string> _history;
	std::string _command_line;

	void startup()
	{
		_app.add_subcommand("exit", "Exit the game")->callback(close_window);
		_app.add_subcommand("clear", "Clear the console")->callback(clear);

		// MAPS
		{
			auto cmd = _app.add_subcommand("map", "Handle maps");
			cmd->add_option_function<std::string>("-o,--open", map::open, "Open a map");
			cmd->add_flag_callback("-c,--close", map::close, "Close the current map");
			cmd->footer("Example: map -o dungeon.tmx");
		}

		log("Type -h or --help for a list of commands");
	}

	void update()
	{
        if (!_visible)
			return;

		
		if (ImGui::Begin("Console"))
		{
			ImGui::SetWindowFontScale(2.f);
			// HISTORY
			{
				// Height of 1 separator + 1 input text
				float command_line_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
				ImVec2 child_window_height(0, -command_line_height);
				int child_window_flags = ImGuiWindowFlags_HorizontalScrollbar;
				if (ImGui::BeginChild("History", child_window_height, false, child_window_flags))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
					for (const auto& line : _history)
						ImGui::TextUnformatted(line.c_str());
					ImGui::PopStyleVar();
				}
				ImGui::EndChild();
			}

			ImGui::Separator();

            // COMMAND LINE

			ImGui::PushItemWidth(-1); // Use all available width
			if (ImGui::InputText("##CommandLine", &_command_line, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				_history.push_back(_command_line);
				execute(_command_line);
				_command_line.clear();
				_reclaim_focus = true;
			}
			ImGui::PopItemWidth();

			// AUTOFOCUS

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
		_history.push_back(message);
	}

	void execute(const std::string& command_line)
	{
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
			log(parse_error.what());
		}
	}
}
