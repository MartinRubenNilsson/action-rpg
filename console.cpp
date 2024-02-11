#include "console.h"
#include <deque>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace console
{
	const ImColor _COLOR_COMMAND = IM_COL32(255, 255, 255, 255);
	const ImColor _COLOR_ERROR = IM_COL32(220, 50, 47, 255);
	const size_t _MAX_HISTORY = 512;
	bool _visible = false;
	bool _reclaim_focus = false;
	float _sleep_timer = 0.f;
	std::stringstream _cout_stream;
	std::stringstream _cerr_stream;
	std::string _command_line;
	std::deque<std::string> _command_queue;
	std::vector<std::string> _command_history;
	std::vector<std::string>::iterator _command_history_it = _command_history.end();
	std::deque<std::pair<std::string, ImColor>> _history;
	std::unordered_map<sf::Keyboard::Key, std::string> _key_bindings;

	// Handles command history navigation
	int _input_text_callback(ImGuiInputTextCallbackData* data)
	{
		if (_command_history.empty()) return 0;
		if (data->EventKey == ImGuiKey_UpArrow) {
			if (_command_history_it > _command_history.begin()) {
				_command_history_it--;
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, _command_history_it->c_str());
			}
		} else if (data->EventKey == ImGuiKey_DownArrow) {
			if (_command_history_it < _command_history.end() - 1) {
				_command_history_it++;
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, _command_history_it->c_str());
			}
		}
		return 0;
	}

	void initialize()
	{
		// REDIRECT COUT AND CERR

		std::cout.rdbuf(_cout_stream.rdbuf());
		std::cerr.rdbuf(_cerr_stream.rdbuf());

		// SETUP IMGUI STYLE
		{
			// https://github.com/ocornut/imgui/issues/707#issuecomment-252413954

			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 5.3f;
			style.FrameRounding = 2.3f;
			style.ScrollbarRounding = 0;

			style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
		}
	}

	void update(float dt)
	{
		// LOG COUT AND CERR
		{
			std::string line;
			while (std::getline(_cout_stream, line)) log(line);
			_cout_stream.str(std::string());
			while (std::getline(_cerr_stream, line)) log_error(line);
			_cerr_stream.str(std::string());
		}

		// UPDATE SLEEP TIMER

		if (_sleep_timer > 0.f) {
			_sleep_timer -= dt;
			if (_sleep_timer < 0.f)
				_sleep_timer = 0.f;
		}

		// EXECUTE COMMAND QUEUE

		while (!_sleep_timer && !_command_queue.empty()) {
			execute(_command_queue.front());
			_command_queue.pop_front();
		}

		// SHOW CONSOLE WINDOW

		if (!_visible) return;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::Begin("Console", &_visible, window_flags)) {
			// HISTORY
			{
				float reserved_height = ImGui::GetStyle().ItemSpacing.y +
					ImGui::GetFrameHeightWithSpacing();
				if (ImGui::BeginChild(
					"History",
					ImVec2(0, -reserved_height), // Leave room for 1 separator + 1 input text
					false,
					ImGuiWindowFlags_HorizontalScrollbar)) {
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
				_input_text_callback)) {
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

	void process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::KeyPressed) {
			auto it = _key_bindings.find(event.key.code);
			if (it != _key_bindings.end())
				execute(it->second);
		}
	}

	bool is_visible() {
		return _visible;
	}

	void set_visible(bool visible)
	{
		_visible = visible;
		if (_visible) _reclaim_focus = true;
	}

	void toggle_visible()
	{
		_visible = !_visible;
		if (_visible) _reclaim_focus = true;
	}

	void clear() {
		_history.clear();
	}

	void sleep(float seconds) {
		_sleep_timer = seconds;
	}

	void log(const std::string& message)
	{
		_history.emplace_back(message, ImGui::GetStyle().Colors[ImGuiCol_Text]);
		if (_history.size() > _MAX_HISTORY)
			_history.pop_front();
	}

	void log_error(const std::string& message, bool show_console)
	{
		_history.emplace_back(message, _COLOR_ERROR);
		if (_history.size() > _MAX_HISTORY)
			_history.pop_front();
		if (show_console) _visible = true;
	}

	// Defined in console_commands.cpp
	extern void _do_execute(const std::string& command_line);

	void execute(const std::string& command_line, bool defer)
	{
		if (command_line.starts_with("//"))
			return; // ignore comments
		if (defer) {
			_command_queue.push_back(command_line);
			return;
		}
		_command_history.push_back(command_line);
		_command_history_it = _command_history.end();
		_history.emplace_back(command_line, _COLOR_COMMAND);
		_do_execute(command_line);
	}

	void execute(int argc, char* argv[])
	{
		if (argc == 1) return;
		std::string command_line;
		for (int i = 1; i < argc; ++i) {
			command_line += argv[i];
			if (i < argc - 1)
				command_line += ' ';
		}
		execute(command_line);
	}

	void execute_script(const std::string& script_name)
	{
		std::filesystem::path script_path = "assets/scripts/" + script_name;
		script_path.replace_extension(".script");
		std::ifstream script_file(script_path);
		if (!script_file) {
			log_error("Failed to open script: " + script_path.generic_string());
			return;
		}
		std::string command_line;
		while (std::getline(script_file, command_line))
			execute(command_line, true);
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
