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
	std::unordered_map<sf::Keyboard::Key, std::string> _key_bindings;
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

	void initialize(sf::RenderWindow& window)
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

		_app.set_help_flag("-h,--help", "Print a help message");
		_app.set_help_all_flag("--help-all", "Print help messages for all subcommands");

		// SETUP CONSOLE COMMANDS
		{
			_app.add_subcommand("clear", "Clear the console")
				->callback(clear);
			_app.add_subcommand("script", "Execute a console script")
				->add_option_function<std::string>("name", execute_script, "The name of the script")
				->required();
			{
				static std::string key_string;
				static std::string command_line;
				auto bind_cmd = _app.add_subcommand("bind", "Bind a key to a console command");
				bind_cmd->add_option("key", key_string, "The key to bind")
					->required();
				bind_cmd->add_option("command", command_line, "The command to execute")
					->required();
				bind_cmd->callback([](){ console::bind(key_string, command_line); });
			}
			{
				static std::string key_string;
				auto bind_cmd = _app.add_subcommand("unbind", "Unbind a key");
				bind_cmd->add_option("key", key_string, "The key to unbind")
					->required();
				bind_cmd->callback([]() { console::unbind(key_string); });
			}
		}

		// SETUP WINDOW COMMANDS
		{
			auto window_cmd = _app.add_subcommand("window", "Manage the window");
			{
				window_cmd->add_subcommand("close", "Close the window")
					->callback([&window]() { window.close(); });
			}
			{
				auto set_title_cmd = window_cmd->add_subcommand("set_title", "Set the window title");
				static std::string title;
				set_title_cmd->add_option("title", title, "The new title of the window")
					->required();
				set_title_cmd->callback([&window]() { window.setTitle(title); });
			}
			{
				auto set_size_cmd = window_cmd->add_subcommand("set_size", "Set the window size");
				static sf::Vector2u size;
				set_size_cmd->add_option("x", size.x, "The new width of the window")
					->required();
				set_size_cmd->add_option("y", size.y, "The new height of the window")
					->required();
				set_size_cmd->callback([&window]() { window.setSize(size); });
			}
		}

		// SETUP OTHER COMMANDS
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

	void process_event(const sf::Event& event) {
		if (event.type == sf::Event::KeyPressed && _key_bindings.contains(event.key.code))
			execute(_key_bindings[event.key.code]);
	}

	void toggle_visible()
	{
		_visible = !_visible;
		if (_visible)
			_reclaim_focus = true;
	}

	void clear() {
		_history.clear();
	}

	void log(const std::string& message) {
		_history.emplace_back(message, ImGui::GetStyle().Colors[ImGuiCol_Text]);
	}

	void log_error(const std::string& message, bool make_visible)
	{
		_history.emplace_back(message, _red);
		if (make_visible) _visible = true;
	}

	void execute(const std::string& command_line)
	{
		if (command_line.starts_with("//"))
			return; // ignore commented commands

		_command_history.push_back(command_line);
		_command_history_it = _command_history.end();
		_history.emplace_back(command_line, _blue);

		try
		{
			_app.parse(command_line); // throws
		}
		catch (const CLI::CallForHelp&) // thrown by -h or --help
		{
			log(_app.help());
		}
		catch (const CLI::CallForAllHelp&) // thrown by --help-all
		{
			log(_app.help("", CLI::AppFormatMode::All));
		}
		catch (const CLI::ParseError& parse_error)
		{
			log_error(parse_error.what());
		}
	}

	std::string _implode(
		const std::vector<std::string>& strings,
		const std::string& separator)
	{
		std::string result;
		for (size_t i = 0; i < strings.size(); ++i)
		{
			result += strings[i];
			if (i < strings.size() - 1)
				result += separator;
		}
		return result;
	}

	void execute_script(const std::string& script_name)
	{
		static std::vector<std::string> stack;
		if (std::find(stack.begin(), stack.end(), script_name) != stack.end())
		{
			log_error("Script recursion detected: " + _implode(stack, " -> "));
			return;
		}
		stack.push_back(script_name);
		{
			std::filesystem::path script_path = "assets/scripts/" + script_name;
			script_path.replace_extension(".script");
			std::ifstream script_file(script_path);
			if (!script_file)
			{
				log_error("Failed to open script: " + script_path.generic_string());
				return;
			}
			std::string line;
			while (std::getline(script_file, line))
				execute(line);
		}
		stack.pop_back();
	}

	void bind(sf::Keyboard::Key key, const std::string& command_line) {
		_key_bindings[key] = command_line;
	}

	void bind(const std::string& key_string, const std::string& command_line)
	{
		auto key = magic_enum::enum_cast<sf::Keyboard::Key>(
			key_string, magic_enum::case_insensitive);
		if (key.has_value()) bind(key.value(), command_line);
		else log_error("Failed to bind key: " + key_string);
	}

	void unbind(sf::Keyboard::Key key) {
		_key_bindings.erase(key);
	}

	void unbind(const std::string& key_string)
	{
		auto key = magic_enum::enum_cast<sf::Keyboard::Key>(
			key_string, magic_enum::case_insensitive);
		if (key.has_value()) unbind(key.value());
		else log_error("Failed to unbind key: " + key_string);
	}
}