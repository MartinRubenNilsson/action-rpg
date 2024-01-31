#pragma once

namespace ui
{
	enum class Request
	{
		None,
		Play,
		Quit,
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
	bool should_pause_game();
	Request get_next_request();

	std::vector<std::string> get_document_names();
	void show_document(const std::string& name);
	void hide_document(const std::string& name);
}
