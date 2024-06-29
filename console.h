#pragma once

namespace window
{
	enum class Key;
	struct Event;
}

namespace console
{
	void initialize();
	void update(float dt); // Must come after ImGui::SFML::Update but before Imgui::SFML::Render.
	void process_window_event(const window::Event& ev);

	bool is_visible();
	void set_visible(bool visible);
	void toggle_visible();
	bool has_focus();
	void clear();
	void sleep(float seconds);
	void log(const std::string& message);
	void log_error(const std::string& message, bool show_console = true);
	void execute(const std::string& command_line, bool defer = false);
	void execute(int argc, char* argv[]);
	// Scripts are kept in the assets/scripts folder.
	// The script name should not include the .script extension.
	void execute_script(const std::string& script_name);
	void bind(window::Key key, const std::string& command_line);
	void bind(const std::string& key_string, const std::string& command_line);
	void unbind(window::Key key);
	void unbind(const std::string& key_string);
}
