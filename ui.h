#pragma once

namespace window
{
	struct Event;
}

namespace ui
{
	enum class EventType
	{
		PlayGame,
		RestartMap,
		QuitApp,
		GoToMainMenu,
	};

	struct Event
	{
		EventType type;
	};

	namespace bindings
	{
		void on_click_play();
		void on_click_settings();
		void on_click_credits();
		void on_click_quit();
		void on_click_back();
		void on_click_resume();
		void on_click_restart();
		void on_click_main_menu();
	}

	extern bool debug;

	void initialize();
	void shutdown();
	void process_window_event(const window::Event& ev);
	void update(float dt);
	void render();

	void load_font_from_file(const std::string& path);
	void load_document_from_file(const std::string& path);
	void add_event_listeners();

	// Reloads all documents' style sheets from <style> tags
	// and external style sheets, but not inline "style" attributes.
	void reload_styles();
	void show_document(const std::string& name);

	bool get_next_event(Event& ev);
	bool is_menu_or_textbox_visible();
}
