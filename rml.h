#pragma once

namespace rml
{
	void initialize(sf::RenderWindow* window);
	void shutdown();
	void process_event(const sf::Event& event);
	void update();
	void render(); // Uses OpenGL, so push and pop the OpenGL states before and after calling this function.

	// Loads all TTF fonts in the assets/fonts folder,
	// and all RML documents in the assets/rml folder.
	void load_fonts_and_documents();

	// Returns a list of names of all loaded documents.
	std::vector<std::string> get_list();

	// Shows the document with the given name.
	void show(const std::string& name);

	// Hides the document with the given name.
	void hide(const std::string& name);
}
