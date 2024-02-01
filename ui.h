#pragma once

namespace ui
{
	enum class Event
	{
		None,
		PlayGame,
		QuitApp,
		GoToMainMenu,
	};

	void initialize(sf::RenderWindow& window);
	void shutdown();
	void load_ttf_fonts(const std::filesystem::path& dir);
	void load_rml_documents(const std::filesystem::path& dir);
	// Reloads all documents' style sheets from <style> tags
	// and external style sheets, but not inline "style" attributes.
	void reload_styles();

	bool process_event(const sf::Event& ev);
	void update(float dt);
	void render(); // Uses OpenGL, so make sure to call resetGLStates() after.

	Event poll_event();
	bool should_pause_game();
}
