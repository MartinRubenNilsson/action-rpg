#pragma once

namespace ui
{
	struct Event
	{
		enum Type
		{
			PlayGame,
			RestartMap,
			QuitApp,
			GoToMainMenu,
		} type;
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
	void load_ttf_fonts(const std::filesystem::path& dir);
	void load_rml_documents(const std::filesystem::path& dir);
	void add_event_listeners();
	void process_window_event(const sf::Event& ev);
	void update(float dt);
	void render(sf::RenderTarget& target);

	// Reloads all documents' style sheets from <style> tags
	// and external style sheets, but not inline "style" attributes.
	void reload_styles();
	void show_document(const std::string& name);

	bool poll_event(Event& ev);
	bool should_pause_game();
}
