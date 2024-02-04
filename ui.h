#pragma once

namespace ui
{
	enum class Event
	{
		None,
		PlayGame,
		RestartMap,
		QuitApp,
		GoToMainMenu,
	};

	extern bool debug;

	void initialize();
	void shutdown();
	void load_ttf_fonts(const std::filesystem::path& dir);
	void load_rml_documents(const std::filesystem::path& dir);
	void add_event_listeners();
	void process_event(const sf::Event& ev);
	void update(float dt);
	void render(sf::RenderTarget& target);

	// Reloads all documents' style sheets from <style> tags
	// and external style sheets, but not inline "style" attributes.
	void reload_styles();
	void show_document(const std::string& name);

	Event poll_event();
	bool should_pause_game();
}
