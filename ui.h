#pragma once

namespace ui
{
	void initialize(sf::RenderWindow& window);
	void shutdown();
	void process_event(const sf::Event& event);
	void update(float dt);
	void render(); // Uses OpenGL, so make sure to call resetGLStates() after.
	bool should_pause_game();

	// Loads all TTF fonts in the assets/fonts folder,
	// and all RML documents in the assets/ui folder.
	void load_fonts_and_documents();

	// Reloads all documents' style sheets from <style> tags
	// and external style sheets, but not inline "style" attributes.
	void reload_styles();

	// Returns a list of names of all loaded documents.
	std::vector<std::string> get_document_names();

	// Shows the document with the given name.
	void show(const std::string& document_name);

	// Hides the document with the given name.
	void hide(const std::string& document_name);
}
