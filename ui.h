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

	void push_event(const Event& ev);
	bool poll_event(Event& ev);
	bool should_pause_game();
}
